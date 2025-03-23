/**************************************************************
 * Wii Nunchuk linux driver.
 * Presents the Wii Nunchuk as a linux input gamepad.
 * 
 * Author: Tobi Ogundimu
 * E-mail: tobiogundimu@gmail.com
 *************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/of_device.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/errno.h>

#define CLIENT_ADDR 0x52

MODULE_AUTHOR("Tobi Ogundimu <tobiogundimu@gmail.com>");
MODULE_DESCRIPTION("Driver for the Nintendo Wii Nunchuk.");
MODULE_LICENSE("GPL");

static struct of_device_id dev_ids[] = {
	{
		.compatible = "nintendo,wiinunchuk",
	},
	{}
};
MODULE_DEVICE_TABLE(of, dev_ids);

static struct i2c_device_id i2c_dev_ids[] = {
	{
		.name = "Nintendo Wii Nunchuk",
	},
	{}
};
MODULE_DEVICE_TABLE(i2c, i2c_dev_ids);

static struct input_dev* nnchk_input;
static int nnchk_open(struct input_dev* dev);
static void nnchk_close(struct input_dev*dev);

static struct i2c_client* nnchk_client;
static int nnchk_probe(struct i2c_client* client);
static struct i2c_driver nnchk_drv = {
	.driver = {
		.name = "wii nunchuk",
		.of_match_table = dev_ids,
		.owner = THIS_MODULE,
	},
	.id_table = i2c_dev_ids,
	.probe = nnchk_probe,
};

static struct task_struct* poll_task;
static int nnchk_poll(void* data);

/**********************************************************************
 * Set up I2C device and virtual input device
 *********************************************************************/
static int __init nnchk_init(void){
	if(i2c_add_driver(&nnchk_drv)){
		printk("I2C driver add function failed.\n");
		return -1;
	}
	nnchk_input = input_allocate_device();
	nnchk_input -> name = "Wii Nunchuk";
	nnchk_input -> evbit[0] = BIT_MASK(EV_KEY)|BIT_MASK(EV_ABS);
	nnchk_input -> keybit[BIT_WORD(BTN_GAMEPAD)] = BIT_MASK(BTN_GAMEPAD)|BIT_MASK(BTN_TL)|BIT_MASK(BTN_TL2);
	nnchk_input -> absbit[0] = BIT_MASK(ABS_X)|BIT_MASK(ABS_Y)|BIT_MASK(ABS_Z)|BIT_MASK(ABS_TILT_X)|BIT_MASK(ABS_TILT_Y);
	input_set_abs_params(nnchk_input, ABS_X, 0, 255, 4, 8);
	input_set_abs_params(nnchk_input, ABS_Y, 0, 255, 4, 8);
	input_set_abs_params(nnchk_input, ABS_Z, 0, 1023, 4, 8);
	input_set_abs_params(nnchk_input, ABS_TILT_X, 0, 1023, 4, 8);
	input_set_abs_params(nnchk_input, ABS_TILT_Y, 0, 1023, 4, 8);
	nnchk_input -> open = nnchk_open;
	nnchk_input -> close = nnchk_close;
	nnchk_input -> hint_events_per_packet = 7;

	if(input_register_device(nnchk_input)){
		printk(KERN_ERR "Failed to register device.\n");
		goto free_input_device;
	}
	return 0;

free_input_device:
	input_free_device(nnchk_input);
	i2c_del_driver(&nnchk_drv);
	return -1;
}
module_init(nnchk_init);

static void __exit nnchk_exit(void){
	input_unregister_device(nnchk_input);
	i2c_del_driver(&nnchk_drv);
}
module_exit(nnchk_exit);

static int nnchk_probe(struct i2c_client* client){
	if(client -> addr != CLIENT_ADDR){
		//Wii Nunchuk address is always 0x52.
		printk("Error: Address must be 0x52.\n");
		return -1;
	}
	nnchk_client = client;
	return 0;
}

/***********************************************************************
 * Begin polling thread when event file is accessed.
 **********************************************************************/
static int nnchk_open(struct input_dev* dev){
	poll_task = kthread_run(nnchk_poll, NULL, "Nunchuk poll thread");
	if(IS_ERR(poll_task)){
		printk(KERN_ERR "Unable to create kthread.\n");
		return -1;
	}
	return 0;
}

/***********************************************************************
 * Stop polling when event file is closed.
 **********************************************************************/
static void nnchk_close(struct input_dev* dev){
	kthread_stop(poll_task);
}

/**********************************************************************
 * Polling task: The wii nunchuk has no interrupt signals,
 * so data is retrieved through polling.
 *********************************************************************/
static int nnchk_poll(void* data){
	unsigned char buffer[6];
	unsigned short interval = 0;//Nunchuk needs to be reinitialized frequently
	int tmp;
	unsigned int acc_X;
	unsigned int acc_Y;
	unsigned int acc_Z;
	char handshake[2] = {0x40, 0x00};
	while(!kthread_should_stop()){
		//Send handshake signal regularly to prevent sleeping.
		if(interval % 10 == 0){
			tmp = i2c_master_send(nnchk_client, handshake, 2);
			if(tmp < 2) printk("Handshake incomplete.\n");
		}
		interval++;
		fsleep(1000);//delays are required to allow the nunchuk to fetch data.
		if(i2c_smbus_write_byte(nnchk_client,0x00) < 0)
			printk("Request for data failed");

		fsleep(1000);
		tmp = i2c_master_recv(nnchk_client, buffer, 6);
		if(tmp < 6) printk("Data incomplete.\n");
		
		for(int i = 0; i < 6; i++) 
			buffer[i] = (buffer[i] ^ 0x17) + 0x17;//Data from the wii nunchuk must be decrypted.
			
		/**********************************************************************
		 * The two least significant bits of the accelerometers are stored
		 * in the 6th byte.
		 *********************************************************************/
		acc_X = (buffer[2] << 2) | ((buffer[5] >> 2) & 0x03);
		acc_Y = (buffer[3] << 2) | ((buffer[5] >> 4) & 0x03);
		acc_Z = (buffer[4] << 2) | ((buffer[5] >> 6) & 0x03);
		
		input_report_key(nnchk_input, BTN_TL, buffer[5] & 0x02);
		input_report_key(nnchk_input, BTN_TL2, buffer[5] & 0x01);
		input_report_abs(nnchk_input, ABS_X, buffer[0]);
		input_report_abs(nnchk_input, ABS_Y, buffer[1]);
		input_report_abs(nnchk_input, ABS_TILT_X, acc_X);
		input_report_abs(nnchk_input, ABS_TILT_Y, acc_Y);
		input_report_abs(nnchk_input, ABS_Z, acc_Z);
		input_sync(nnchk_input);
		msleep(14);
	}
	return 0;
}
