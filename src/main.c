#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include <device.h>
#include <drivers/i2c.h>

#include "logging/log.h"

#define DEVICE_NAME "SmartSensor"
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static volatile u8_t mfg_data[] = { 0x00, 0x00, 0xaa, 0xbb };

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, 4)
};

/* Set Scan Response data */
static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_SHORTENED, DEVICE_NAME, DEVICE_NAME_LEN),
};

static struct device* dev_si7021;

static void bt_ready(int err)
{
	if (err) {
		return;
	}

	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		return;
	}

}


void update_sensor_data()
{

    struct sensor_value temp,humidity;

    sensor_sample_fetch(dev_si7021);
    sensor_channel_get(dev_si7021, SENSOR_CHAN_AMBIENT_TEMP, &temp);	
    sensor_channel_get(dev_si7021, SENSOR_CHAN_HUMIDITY, &humidity);

	printk("Temp: %dC	Hum : %%%d\n",temp.val1,humidity.val1);


	mfg_data[2] = (uint8_t) temp.val1;
	mfg_data[3] = (uint8_t) humidity.val1;
	
}

void main(void)
{
	int err;
	
	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
	}

	dev_si7021 = device_get_binding("SI7006");

	if (dev_si7021 == NULL) {
		printk("No device found; did initialization fail?\n");
		return;
	}

	while(1)
	{
		
		update_sensor_data();
		bt_le_adv_update_data(ad, ARRAY_SIZE(ad), NULL, 0);
		k_sleep(2000);

	}

}