#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <errno.h>

#define I2C_TARGET_ADDR 0x42

#define REG_LED2_CONTROL 0x00
#define REG_LED2_STATUS  0x01
#define REG_DEVICE_ID    0x02
#define REG_COUNT        16

#define DEVICE_ID        0x47

#define LED0_NODE DT_ALIAS(led0)

#if !DT_NODE_HAS_STATUS(LED0_NODE, okay)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static uint8_t registers[REG_COUNT] = {
	[REG_LED2_CONTROL] = 0x00,
	[REG_LED2_STATUS]  = 0x00,
	[REG_DEVICE_ID]    = DEVICE_ID,
};

static uint8_t register_index;
static bool expecting_register_index;

static void led2_set(uint8_t value)
{
	int led_on = value ? 1 : 0;

	gpio_pin_set_dt(&led2, led_on);

	registers[REG_LED2_CONTROL] = led_on;
	registers[REG_LED2_STATUS] = led_on;

	printk("LED2 %s\n", led_on ? "ON" : "OFF");
}

/*
 * I2C-controller/master starter en write-transaktion.
 */
static int target_write_requested(struct i2c_target_config *config)
{
	ARG_UNUSED(config);

	expecting_register_index = true;

	printk("I2C write requested\n");

	return 0;
}

/*
 * Første byte fra masteren bruges som register-adresse.
 * Efterfølgende bytes skrives til registeret.
 */
static int target_write_received(struct i2c_target_config *config, uint8_t val)
{
	ARG_UNUSED(config);

	if (expecting_register_index) {
		register_index = val % REG_COUNT;
		expecting_register_index = false;

		printk("Register index set to 0x%02x\n", register_index);
		return 0;
	}

	printk("Write: reg[0x%02x] = 0x%02x\n", register_index, val);

	switch (register_index) {
	case REG_LED2_CONTROL:
		led2_set(val);
		break;

	case REG_DEVICE_ID:
		printk("REG_DEVICE_ID is read-only\n");
		break;

	default:
		registers[register_index] = val;
		break;
	}

	register_index++;
	register_index %= REG_COUNT;

	return 0;
}

/*
 * Masteren starter en read-transaktion.
 */
static int target_read_requested(struct i2c_target_config *config, uint8_t *val)
{
	ARG_UNUSED(config);

	*val = registers[register_index];

	printk("Read requested: reg[0x%02x] -> 0x%02x\n", register_index, *val);

	register_index++;
	register_index %= REG_COUNT;

	return 0;
}

/*
 * Flere bytes læses i samme read-transaktion.
 */
static int target_read_processed(struct i2c_target_config *config, uint8_t *val)
{
	ARG_UNUSED(config);

	*val = registers[register_index];

	printk("Read processed: reg[0x%02x] -> 0x%02x\n", register_index, *val);

	register_index++;
	register_index %= REG_COUNT;

	return 0;
}

/*
 * STOP condition på I2C-bussen.
 */
static int target_stop(struct i2c_target_config *config)
{
	ARG_UNUSED(config);

	printk("I2C stop\n");

	return 0;
}

static void target_error(struct i2c_target_config *config, enum i2c_error_reason reason)
{
	ARG_UNUSED(config);

	printk("I2C error: %d\n", reason);
}

static const struct i2c_target_callbacks target_callbacks = {
	.write_requested = target_write_requested,
	.write_received = target_write_received,
	.read_requested = target_read_requested,
	.read_processed = target_read_processed,
	.stop = target_stop,
	.error = target_error,
};

static struct i2c_target_config target_config = {
	.address = I2C_TARGET_ADDR,
	.callbacks = &target_callbacks,
};

int main(void)
{
	const struct device *i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
	int ret;

	printk("NUCLEO-G474RE I2C target with LED2 register control\n");
	printk("I2C target address: 0x%02x\n", I2C_TARGET_ADDR);
	printk("Register 0x00 controls LED2\n");
	printk("Write 0x00 to turn LED2 OFF\n");
	printk("Write 0x01 to turn LED2 ON\n");

	if (!device_is_ready(i2c_dev)) {
		printk("ERROR: I2C device is not ready\n");
		return 0;
	}

	if (!gpio_is_ready_dt(&led2)) {
		printk("ERROR: LED2 GPIO device is not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("ERROR: Could not configure LED2 GPIO: %d\n", ret);
		return 0;
	}

	led2_set(0);

	ret = i2c_target_register(i2c_dev, &target_config);
	if (ret < 0) {
		printk("ERROR: i2c_target_register failed: %d\n", ret);
		return 0;
	}

	printk("I2C target registered on I2C1\n");

	while (1) {
		k_sleep(K_SECONDS(5));
		printk("Alive. LED2 status: %u\n", registers[REG_LED2_STATUS]);
	}

	return 0;
}