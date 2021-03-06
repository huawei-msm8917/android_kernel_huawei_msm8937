/*
 * goodix_ts_test.c - TP test
 *
 * Copyright (C) 2015 - 2016 Goodix Technology Incorporated   
 * Copyright (C) 2015 - 2016 Yulong Cai <caiyulong@goodix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be a reference
 * to you, when you are integrating the GOODiX's CTP IC into your system,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 */
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include "goodix_ts.h"

#define DRV_NUM_OFFSET	(0x8062 - 0x8047 )
#define SEN_NUM_OFFSET (0x8064 - 0x8047 )
#define KEY_NUM_OFFSET	(0x80A3 - 0x8050)
#define MODULE_SWITCH_DK_OFFSET	(0x8057 - 0x8050)
#define MODULE_SWITCH_SK_OFFSET	(0x805A - 0x8050)

#define MAX_DRV_NUM		32
#define MAX_SEN_NUM		32
#define FLOAT_AMPLIFIER 1000
#define PACKAGE_SIZE    256
#define GTP_SHORTCIRCUT_TEST_IMAMGE_BUFF_MAX (1024*6)

#define Zeusis_TouchKey_Panel 1
/*----------------------------------error_type--------------------------*/

#define INI_FILE_OPEN_ERR       ((0x1<<0)|0x80000000)

#define INI_FILE_READ_ERR       ((0x2<<0)|0x80000000)
#define INI_FILE_ILLEGAL        ((0x4<<0)|0x80000000)

		      // -2147483640
#define SHORT_TEST_ERROR        ((0x8<<0)|0x80000000)
#define CFG_FILE_OPEN_ERR       ((0x1<<4)|0x80000000)
#define CFG_FILE_READ_ERR       ((0x2<<4)|0x80000000)
#define CFG_FILE_ILLEGAL        ((0x4<<4)|0x80000000)
#define UPDATE_FILE_OPEN_ERR    ((0x1<<8)|0x80000000)
#define UPDATE_FILE_READ_ERR    ((0x2<<8)|0x80000000)
#define UPDATE_FILE_ILLEGAL     ((0x4<<8)|0x80000000)
#define FILE_OPEN_CREATE_ERR    ((0x1<<12)|0x80000000)
#define FILE_READ_WRITE_ERR     ((0x2<<12)|0x80000000)
#define FILE_ILLEGAL            ((0x4<<12)|0x80000000)
#define NODE_OPEN_ERR           ((0x1<<16)|0x80000000)
#define NODE_READ_ERR           ((0x2<<16)|0x80000000)
#define NODE_WRITE_ERR          ((0x4<<16)|0x80000000)
#define DRIVER_NUM_WRONG        ((0x1<<20)|0x80000000)
#define SENSOR_NUM_WRONG        ((0x2<<20)|0x80000000)
#define PARAMETERS_ILLEGL       ((0x4<<20)|0x80000000)
#define I2C_UNLOCKED            ((0x8<<20)|0x80000000)
#define MEMORY_ERR              ((0x1<<24)|0x80000000)
#define I2C_TRANS_ERR           ((0x2<<24)|0x80000000)
#define COMFIRM_ERR             ((0x4<<24)|0x80000000)
#define ENTER_UPDATE_MODE_ERR   ((0x8<<24)|0x80000000)
#define VENDOR_ID_ILLEGAL       ((0x1<<28)|0x80000000)
#define STORE_ERROR             ((0x2<<28)|0x80000000)
#define NO_SUCH_CHIP_TYPE       ((0x4<<28)|0x80000000)

#define FILE_NOT_EXIST          ((0x8<<28)|0x80000000)

#define _ERROR(e)      ((0x01 << e) | (0x01 << (sizeof(s32) * 8 - 1)))
#define ERROR          _ERROR(1)	//for common use
//system relevant
#define ERROR_IIC      _ERROR(2)	//IIC communication error.
#define ERROR_MEM      _ERROR(3)	//memory error.

//system irrelevant
#define ERROR_HN_VER   _ERROR(10)	//HotKnot version error.
#define ERROR_CHECK    _ERROR(11)	//Compare src and dst error.
#define ERROR_RETRY    _ERROR(12)	//Too many retries.
#define ERROR_PATH     _ERROR(13)	//Mount path error
#define ERROR_FW       _ERROR(14)
#define ERROR_FILE     _ERROR(15)
#define ERROR_VALUE    _ERROR(16)	//Illegal value of variables

#define _CHANNEL_PASS               0x0000
#define _BEYOND_MAX_LIMIT           0x0001
#define _BEYOND_MIN_LIMIT           0x0002
#define _BEYOND_ACCORD_LIMIT        0x0004
#define _BEYOND_OFFSET_LIMIT        0x0008
#define _BEYOND_JITTER_LIMIT        0x0010
#define _SENSOR_SHORT               0x0020
#define _DRIVER_SHORT               0x0040
#define _COF_FAIL                   0x0080
#define _I2C_ADDR_ERR               0x0100
#define _CONFIG_MSG_WRITE_ERR       0x0200
#define _GET_RAWDATA_TIMEOUT        0x0400
#define _GET_TEST_RESULT_TIMEOUT    0x0800
#define _KEY_BEYOND_MAX_LIMIT       0x1000
#define _KEY_BEYOND_MIN_LIMIT       0x2000
#define _INT_ERROR                  0x4000
#define _TEST_NG                    0x00008000
#define _VERSION_ERR                0x00010000
#define _RESET_ERR                  0x00020000
#define _CURRENT_BEYOND_MAX_LIMIT   0x00040000
#define _MODULE_TYPE_ERR            0x00080000
#define _MST_ERR                    0x00100000
#define _NVRAM_ERR                  0x00200000
#define _GT_SHORT                   0x00400000
#define _BEYOND_UNIFORMITY_LIMIT    0x00800000
#define _BETWEEN_ACCORD_AND_LINE    0x40000000

/*------------------------------------ SHORT TEST PART--------------------------------------*/
#define _bRW_MISCTL__SRAM_BANK          0x4048
#define _bRW_MISCTL__PATCH_AREA_EN_  	0x404D
#define _bRW_MISCTL__MEM_CD_EN          0x4049
#define _bRW_MISCTL__CACHE_EN           0x404b
#define _bRW_MISCTL__TMR0_EN            0x40b0
#define _rRW_MISCTL__SWRST_B0_          0x4180
#define _bWO_MISCTL__CPU_SWRST_PULSE    0x4184
#define _rRW_MISCTL__BOOTCTL_B0_        0x4190
#define _rRW_MISCTL__BOOT_OPT_B0_       0x4218
#define _bRW_MISCTL__RG_OSC_CALIB       0x4268
#define _rRW_MISCTL__BOOT_CTL_          0x5094	//0x4283
#define _rRW_MISCTL__SHORT_BOOT_FLAG    0x5095	//0x4283

#define SHORT_FILE_PATH		"goodix_shortcircut.bin"

/**
 * struct ts_test_params - test parameters
 * drv_num: touch panel tx(driver) number
 * sen_num: touch panel tx(sensor) number
 * sc_drv_num: screen tx number
 * sc_sen_num: screen rx number
 * key_num: key number
 * max_limits: max limits of rawdata
 * min_limits: min limits of rawdata
 * deviation_limits: channel deviation limits
 * short_threshold: short resistance threshold
 * r_drv_drv_threshold: resistance threshold between drv and drv
 * r_drv_sen_threshold: resistance threshold between drv and sen
 * r_sen_sen_threshold: resistance threshold between sen and sen
 * r_drv_gnd_threshold: resistance threshold between drv and gnd
 * r_sen_gnd_threshold: resistance threshold between sen and gnd
 * avdd_value: avdd voltage value
 */
struct ts_test_params {
	u32 drv_num;
	u32 sen_num;
	u32 sc_drv_num;
	u32 sc_sen_num;
	u32 key_num;

	u16 max_limits[MAX_DRV_NUM * MAX_SEN_NUM];
	u16 min_limits[MAX_DRV_NUM * MAX_SEN_NUM];
	u16 deviation_limits[MAX_DRV_NUM * MAX_SEN_NUM];

	u16 short_threshold;
	u16 r_drv_drv_threshold;
	u16 r_drv_sen_threshold;
	u16 r_sen_sen_threshold;
	u16 r_drv_gnd_threshold;
	u16 r_sen_gnd_threshold;
	u16 avdd_value;
};

/**
 * struct ts_test_rawdata - rawdata structure
 * addr: rawdata address in chip ram
 * data: rawdata buffer, u16, big-endian
 * size: rawdata size
 */
struct ts_test_rawdata {
	u16 addr;
	u16 data[MAX_SEN_NUM * MAX_DRV_NUM];
	u32 size;
};

/**
 * struct goodix_ts_test - main data structrue
 * ts: goodix touch screen data
 * test_config: test mode config data
 * orig_config: original config data
 * test_param: test parameters
 * rawdata: raw data structure
 * basedata: base data 
 * product_id: product id infomation
 * test_result: test result string
 * failed_reason: test failed reason
 */
struct goodix_ts_test {
	struct goodix_ts_data *ts;
	struct goodix_ts_config test_config;
	struct goodix_ts_config orig_config;
	struct ts_test_params test_params;
	struct ts_test_rawdata rawdata;
	struct ts_test_rawdata basedata;
	const char *product_id;

	char test_result[TS_RAWDATA_RESULT_MAX];
	char failed_reason[TS_RAWDATA_RESULT_MAX];
};

struct goodix_ts_fw_data{
	u8 short_fw_data[GTP_SHORTCIRCUT_TEST_IMAMGE_BUFF_MAX];
	int short_fw_size;
	bool fw_exist;
};

struct goodix_ts_fw_data goodix_short_fw_data;
static struct goodix_ts_test *gts_test;

/**
 * goodix_hw_testmode - switch firmware to test mode
 */
static inline int goodix_hw_testmode(struct goodix_ts_test *ts_test)
{
	int ret = -ENODEV;
	u8 buf[1];
	buf[0] = 0x01;

	GTP_INFO("goodix_hw_testmode 0x8040 write 0x01");
	
	if(ts_test->ts->ops.i2c_write)
		ret = ts_test->ts->ops.i2c_write(GTP_REG_CMD, &buf[0], 1);

	usleep(18*1000);
	GTP_INFO("goodix_hw_testmode 0x814E write 0x00");

	buf[0] = 0x00;
	ts_test->ts->ops.i2c_write(GTP_READ_COOR_ADDR, &buf[0], 1);

	GTP_INFO("clear 0x814e 00.");
	
	return ret;
}

/**
 * gt1151_passe_config - resolve tx/rx number and key number
 */
static int gt1151_passe_config(struct goodix_ts_config *test_config,
				struct ts_test_params *test_params)
{
	test_params->drv_num = (test_config->data[DRV_NUM_OFFSET] & 0x1F)
					+ (test_config->data[DRV_NUM_OFFSET + 1] & 0x1F);
	test_params->sen_num = (test_config->data[SEN_NUM_OFFSET] & 0x0F)
					+((test_config->data[SEN_NUM_OFFSET] >>4)&0x0F);

		test_params->sc_drv_num = test_params->drv_num;
		test_params->sc_sen_num = test_params->sen_num;
		test_params->key_num = 0;


	GTP_INFO("tx_num:%d,rx_num:%d,sc_tx_num:%d,sc_rx_num:%d,key_num:%d",
		test_params->drv_num, test_params->sen_num,
		test_params->sc_drv_num, test_params->sc_sen_num,
		test_params->key_num);

	return 0;
}

/**
 * goodix_init_testlimits - get test limits(max/min/...) from dt
 */
static int goodix_init_testlimits(struct goodix_ts_test *ts_test)
{
	struct device_node *device;
	char project_id[20];
	int ret, size, reorder, reorder_st;
	int max_limits_offset, min_limits_offset, deviation_limits_offset;
	int row, column ,i;
	u32 value[8];

	GTP_DEBUG("Get test limits from dt");


	sprintf(project_id, "goodix-sensorid-%u",
						ts_test->ts->hw_info.sensor_id);
	device = of_find_compatible_node(NULL, NULL, project_id);
	if (!device) {
		GTP_ERROR("No chip specific dts: %s, need to parse",
			    project_id);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(device,
					"raw_data_limit", &value[0], 3);
	if (ret < 0) {
		GTP_ERROR("Failed to read raw data limits from dt:%d", ret);
		return ret;
	} else {
		for (i = 0; i < MAX_SEN_NUM * MAX_DRV_NUM; i++) {
			ts_test->test_params.max_limits[i] = (u16)(value[0]);
			ts_test->test_params.min_limits[i] = (u16)(value[1]);
			ts_test->test_params.deviation_limits[i] = (u16)(value[2]);
		}
		GTP_INFO("raw_data_limit:%u %u %u", value[0],
			value[1], value[2]);
	}


	size = of_property_count_u32_elems(device, 
			"special_raw_data_limit");
	if (size > 0 && (size % 4) == 0) {
		int nodes = size / 4;
		u32 *array = kmalloc(size * sizeof(u32), GFP_KERNEL);
		if (!array) {
			GTP_ERROR("Failed to alloc memory");
			return -ENOMEM;
		}

		ret = of_property_read_u32_array(device,
				"special_raw_data_limit", array, size);
		if (ret < 0) {
			GTP_ERROR("Failed to parse special_raw_data_limit");
			kfree(array);
			return ret;
		}

		for (i = 0; i < nodes; i++) {
			row = i / (int)(ts_test->test_params.sc_sen_num);
			column = i % (int)(ts_test->test_params.sc_sen_num);
			reorder = column * ts_test->test_params.sc_drv_num + row;
			reorder_st = reorder * 4;
			max_limits_offset = reorder * 4 + 1;
			min_limits_offset = reorder * 4 + 2;
			deviation_limits_offset = reorder * 4 + 3;
			if (array[reorder_st]  < MAX_SEN_NUM * MAX_DRV_NUM) {
				ts_test->test_params.max_limits[i]=
						(u16)(array[max_limits_offset]);
				ts_test->test_params.min_limits[i] =
						(u16)(array[min_limits_offset]);
				ts_test->test_params.deviation_limits[i] =
						(u16)(array[deviation_limits_offset]);
			} else {
				GTP_ERROR("Failed to reorder special_raw_data_limit");
				break;
			}
		}
		kfree(array);
	}

	ret = of_property_read_u32_array(device,
					"shortcircut_threshold", &value[0], 7);
	if (ret < 0) {
		GTP_ERROR("Failed to read shortcircut threshold from dt:%d", ret);
		return ret;
	} else {
		ts_test->test_params.short_threshold = (u16)(value[0]);
		ts_test->test_params.r_drv_drv_threshold =(u16)( value[1]);
		ts_test->test_params.r_drv_sen_threshold =(u16)( value[2]);
		ts_test->test_params.r_sen_sen_threshold =(u16)( value[3]);
		ts_test->test_params.r_drv_gnd_threshold =(u16)( value[4]);
		ts_test->test_params.r_sen_gnd_threshold =(u16)( value[5]);
		ts_test->test_params.avdd_value = (u16)(value[6]);
		GTP_INFO("shortciurt_threshold:%u %u %u %u %u %u %u",
			value[0], value[1], value[2], value[3], value[4],
			value[5], value[6]);
	}

	return 0;
}

/**
 * goodix_read_origconfig - read original config data
 */
static int goodix_read_origconfig(struct goodix_ts_test *ts_test)
{
	int ret = -ENODEV, cfg_size;

	GTP_DEBUG("Read original config data");

	cfg_size = GTP_CONFIG_ORG_LENGTH;
	if (ts_test->ts->ops.i2c_read) {
		ret = ts_test->ts->ops.i2c_read(GTP_REG_CONFIG_DATA,
				&ts_test->orig_config.data[0], cfg_size);
		if (ret < 0) {
			GTP_ERROR("Failed to read original config data");
			return ret;
		}

		if (ts_test->orig_config.data[EXTERN_CFG_OFFSET] & 0x40)  {
			ret = ts_test->ts->ops.i2c_read(GTP_REG_EXT_CONFIG,
				&ts_test->orig_config.data[cfg_size],
				GTP_CONFIG_EXT_LENGTH);
			if (ret < 0) {
				GTP_ERROR("Failed to read external config data");
				return ret;
			}
			cfg_size += GTP_CONFIG_EXT_LENGTH;
		}

		ts_test->orig_config.size = cfg_size;
		ts_test->orig_config.delay_ms = 200;
		ts_test->orig_config.name = "original_config";
		ts_test->orig_config.initialized = true;
	}

	return ret;
}

/**
 * goodix_tptest_prepare - preparation before tp test
 */
static int goodix_tptest_prepare(struct goodix_ts_test *ts_test)
{
	struct goodix_ts_config *test_config;
	int ret, retry, i;
	u8 buf[1];

	GTP_INFO("TP test preparation");
	ret = of_property_read_string(ts_test->ts->pdev->dev.of_node,
					"product_id", &ts_test->product_id);
	if (ret) {
		GTP_ERROR("Invalid product id");
		return -EINVAL;
	}

	test_config = &ts_test->test_config;
	/* parse test config data form dts */
	ret = -ENODEV;
	if (ts_test->ts->ops.parse_cfg_data)
		ret = ts_test->ts->ops.parse_cfg_data(ts_test->ts,
			"tptest_config", &test_config->data[0], &test_config->size,
			ts_test->ts->hw_info.sensor_id);

	if (ret < 0) {
		GTP_ERROR("Failed to parse tptest-config:%d",ret);
		return ret;
	} else {
		test_config->initialized = true;
		test_config->name = "tptest_config";
		test_config->delay_ms = 40;
	}

	/* parse sensro & driver number */

		gt1151_passe_config(&ts_test->test_config,
						&ts_test->test_params);
		ts_test->rawdata.addr = 0x9B60;
		ts_test->basedata.addr = 0x9560;

	/* get test limits from dt */
	ret = goodix_init_testlimits(ts_test);
	if (ret < 0) {
		GTP_ERROR("Failed to init testlimits from dt:%d", ret);
		return ret;
	}

	ret = goodix_read_origconfig(ts_test);
	if (ret < 0)
		return ret;

	/* same config version, ensure firmware will accept
		this test config data */
	ret = -ENODEV;
	if (ts_test->ts->ops.send_cfg)
		ret = ts_test->ts->ops.send_cfg(test_config);
	if (ret < 0) {
		GTP_ERROR("Failed to send test config:%d", ret);
		return ret;
	} else {
		for (i = 0; i < 3; i++) {
			retry = 40;
			while (retry--) {
				ret = ts_test->ts->ops.i2c_read(GTP_READ_COOR_ADDR, &buf[0], 1);
				if (ret < 0 || (buf[0] & 0x80) == 0x80)
					break;
				usleep_range(5000, 5010);
			}

			if (!ret && (buf[0] & 0x80) == 0x80) {
				buf[0] = 0x00;
				ret = ts_test->ts->ops.i2c_write(GTP_READ_COOR_ADDR, &buf[0], 1);
				if (ret < 0)
					return ret;
			} else if (ret < 0) {
				return ret;	
			}
			GTP_DEBUG("Clear int status{0x814e}");
		}
	}

	return 0;
}

/**
 * goodix_tptest_finish - finish test
 */
static inline int goodix_tptest_finish(struct goodix_ts_test *ts_test)
{
	int ret = -ENODEV;

	GTP_INFO("TP test finish");
	goodix_pinctrl_select_normal(goodix_ts);
	goodix_chip_reset();
	if (ts_test->ts->ops.send_cfg)
		ret = ts_test->ts->ops.send_cfg(&ts_test->orig_config);
	if (ret < 0) {
		GTP_ERROR("Failed to send normal config:%d", ret);
		return ret;
	}

	return 0;
}

/**
 * goodix_cache_rawdata - cache rawdata
 */
static int goodix_cache_rawdata(struct goodix_ts_test *ts_test)
{
	int ret = -1,retry = 40;
	u8 buf[1] = {0x00};
	u32 rawdata_size ,i;

	GTP_DEBUG("Cache rawdata");
	rawdata_size = ts_test->test_params.sc_sen_num *
			ts_test->test_params.sc_drv_num +
			ts_test->test_params.key_num;

	if (rawdata_size > MAX_DRV_NUM * MAX_SEN_NUM) {
		GTP_ERROR("Invalid rawdata size(%u)", rawdata_size);
		return -EINVAL;
	}

	while (retry--) {
		ret = ts_test->ts->ops.i2c_read(GTP_READ_COOR_ADDR, &buf[0], 1);
		if (ret < 0 || (buf[0] & 0x80) == 0x80)
			break;
		usleep_range(5000, 5010);
	}

	if (!ret && (buf[0] & 0x80) == 0x80) {
		/* cache rawdata */
		ret = ts_test->ts->ops.i2c_read(ts_test->rawdata.addr,
				(u8 *)&ts_test->rawdata.data[0], rawdata_size * 2);
		if (ret < 0) {
			GTP_ERROR("Failed to read rawdata:%d", ret);
			return ret;
		}

		for (i = 0; i < rawdata_size; i++)
			ts_test->rawdata.data[i] =
					be16_to_cpu(ts_test->rawdata.data[i]);
		ts_test->rawdata.size = rawdata_size;
		GTP_INFO("Rawdata ready");
	} else {
		GTP_ERROR("Read rawdata timeout");
		ret = -EFAULT;
	}

	buf[0] = 0x00;
	ts_test->ts->ops.i2c_write(GTP_READ_COOR_ADDR, &buf[0], 1);
	return ret;
}

static int goodix_store_noisedata(struct goodix_ts_test *ts_test,
				struct ts_rawdata_info *info)
{
	int ret = -1, retry = 20;
	u8 buf[1] = {0x00};
	u32 data_size, i, used_offset;

	data_size = ts_test->test_params.sc_sen_num *
			ts_test->test_params.sc_drv_num +
			ts_test->test_params.key_num;

	if (data_size > MAX_DRV_NUM * MAX_SEN_NUM) {
		GTP_ERROR("Invalid rawdata size(%u)", data_size);
		return -EINVAL;
	}

	while (retry--) {
		ret = ts_test->ts->ops.i2c_read(GTP_READ_COOR_ADDR, &buf[0], 1);
		if (ret < 0 || (buf[0] & 0x80) == 0x80)
			break;
		usleep_range(5000, 5010);
	}

	if (!ret && (buf[0] & 0x80) == 0x80) {
		ret = ts_test->ts->ops.i2c_read(ts_test->basedata.addr,
				(u8 *)&ts_test->basedata.data[0], data_size * 2);
		if (ret < 0) {
			GTP_ERROR("Failed to read basedata:%d", ret);
			return ret;
		}

		for (i = 0; i < data_size; i++)
			ts_test->basedata.data[i] =
					be16_to_cpu(ts_test->basedata.data[i]);
		ts_test->basedata.size = data_size;

		if (ts_test->basedata.size + info->used_size <=
						TS_RAWDATA_BUFF_MAX) {
			for (i = 0; i < ts_test->basedata.size; i++)
				{
				used_offset =(u32)( info->used_size + i);
				info->buff[used_offset] =(int)
						(ts_test->basedata.data[i] - 
						ts_test->rawdata.data[i]);
				}
			info->used_size += ts_test->basedata.size;
		} else {
			GTP_ERROR("Invalid basedata size[%u]",
						ts_test->basedata.size);
		}
	} else {
		GTP_ERROR("Read basedata timeout");
		ret = -EFAULT;
	}

	buf[0] = 0x00;
	ts_test->ts->ops.i2c_write(GTP_READ_COOR_ADDR, &buf[0], 1);

	return ret;
}

static int goodix_rawcapacitance_test(struct ts_test_rawdata *rawdata,
				struct ts_test_params *test_params)
{
	bool pass = true;
	u32 i;

	for (i = 0; i < rawdata->size; i++) {
		if (rawdata->data[i] > test_params->max_limits[i]) {
			GTP_ERROR("rawdata[%d][%d]:%u > max_limit:%u, NG", 
				i / test_params->sc_sen_num, i % test_params->sc_sen_num,
				rawdata->data[i], test_params->max_limits[i]);
			pass = false;
		}

		if (rawdata->data[i] < test_params->min_limits[i]) {
			GTP_ERROR("rawdata[%d][%d]:%u < mix_limit:%u, NG",
				i / test_params->sc_sen_num, i % test_params->sc_sen_num,
				rawdata->data[i], test_params->min_limits[i]);
			pass = false;
		}
	}

	return pass ? 0 : -1;
}

static int goodix_deltacapacitance_test(struct ts_test_rawdata *rawdata,
				struct ts_test_params *test_params)
{
	u16 max_val = 0, rawdata_val;
	u16 data_up = 0, data_down = 0, data_left = 0, data_right = 0;
	int cols,rawdata_offset;
	u32 sc_data_num;
	bool pass = true;
	int i;

	cols = test_params->sc_sen_num;
	sc_data_num = test_params->sc_drv_num * test_params->sc_sen_num;

		if (cols == 0 || sc_data_num == 0)
			return -EINVAL;
	
	for (i = 0; i < sc_data_num; i++) {
		rawdata_val = rawdata->data[i];
		max_val = 0;
		if (i -cols>= 0) {
			rawdata_offset = i - cols;
			data_up =rawdata->data[rawdata_offset];
			data_up = abs(rawdata_val - data_up);
			if (data_up > max_val)
				max_val = data_up;
		}

		if (i + cols < sc_data_num) {
			rawdata_offset = i + cols;
			data_down = rawdata->data[rawdata_offset];
			data_down = abs(rawdata_val - data_down);
			if (data_down > max_val)
				max_val = data_down;
		}

		if (i % cols) {
			rawdata_offset = i - 1;
			data_left = rawdata->data[rawdata_offset];
			data_left = abs(rawdata_val - data_left);
			if (data_left > max_val)
				max_val = data_left;
		}

		if ((i + 1) % cols) {
			rawdata_offset = i + 1;
			data_right = rawdata->data[rawdata_offset];
			data_right = abs(rawdata_val - data_right);
			if (data_right > max_val)
				max_val = data_right;
		}

		/* float to integer */
		max_val *= FLOAT_AMPLIFIER;
		max_val /= rawdata_val;
		if (max_val > test_params->deviation_limits[i]) {
			GTP_ERROR("deviation[%d][%d]:%u > delta_limit:%u, NG",
			i / cols, i % cols, max_val,
			test_params->deviation_limits[i]);
			pass = false;
		}
	
	}

	return pass ? 0 : -1;
}

static int goodix_capacitance_test(struct goodix_ts_test *ts_test,
				struct ts_rawdata_info *info)
{
	int test_result0 = -1, test_result1 = -1;
	int test_cnt = 0;
	int ret;
	u32 i, rawdata_offset;

begin_test:
	if (test_cnt > 1)
		goto end_test;

	for (i = 0; i < 4; i++) {
		GTP_INFO("RawCap Test:%d:%d", test_cnt, i);
		ret = goodix_cache_rawdata(ts_test);
		if (ret < 0) {
			/* test NG, re-test once */
			test_cnt++;
			test_result0 = -1;
			test_result1 = -1;
			goto begin_test;
		}

		test_result0 = goodix_rawcapacitance_test(&ts_test->rawdata,
						&ts_test->test_params);
		test_result1 = goodix_deltacapacitance_test(&ts_test->rawdata,
						&ts_test->test_params);
		if (test_result0 < 0 || test_result1 < 0) {
			/* test NG, re-test once */
			test_cnt++;
			goto begin_test;
		}
	}

end_test:
	if (!test_result0) {
		/* test OK*/
		strcat(ts_test->test_result, "-1P");
		
	} else {
		strcat(ts_test->test_result, "-1F");
		GTP_ERROR("RawCap test:NG");
	}

	if (!test_result1) {
		/* test OK*/
		strcat(ts_test->test_result, "-2P");
	} else {
		strcat(ts_test->test_result, "-2F");
		GTP_ERROR("DeltaCap test:NG");
	}

	/* save rawdata to info->buff */
	if (ts_test->rawdata.size <= TS_RAWDATA_BUFF_MAX) {
		info->buff[0] =(int)( ts_test->test_params.sen_num);
		info->buff[1] =(int)( ts_test->test_params.drv_num);
		for (i = 0; i < ts_test->rawdata.size; i++)
			{
			rawdata_offset = i + 2;
			info->buff[rawdata_offset] =(int)( ts_test->rawdata.data[i]);
			}
		info->used_size =(int)( ts_test->rawdata.size) + 2;
	} else {
		GTP_ERROR("Invalid rawdata size[%u]",
				ts_test->rawdata.size);
	}

	return 0;
}

static int goodix_shortcircut_test(struct goodix_ts_test *ts_test);

int goodix_get_rawdata(struct ts_rawdata_info *info,
				struct ts_cmd_node *out_cmd)
{
	int ret = 0;

	if (!goodix_ts)
		return -ENODEV;

	if (!gts_test) {
		gts_test = kzalloc(sizeof(struct goodix_ts_test),
						GFP_KERNEL);
		if (!gts_test) {
			GTP_ERROR("Failed to alloc mem");
			return -ENOMEM;
		}
		gts_test->ts = goodix_ts;
	}
	goodix_ts->enter_rawtest = true;
	ret = goodix_tptest_prepare(gts_test);
	if (ret < 0) {
		GTP_ERROR("TP test init error:%d", ret);
		strcpy(gts_test->test_result, "0F-1F-2F-3F");
		goto exit_finish;
	}

	/* i2c test and switch hw working mode */
	ret = goodix_hw_testmode(gts_test);
	if (ret < 0) {
		GTP_ERROR("Unable to set hw testmode:%d", ret);
		strcpy(gts_test->test_result, "0F-1F-2F-3F");
		goto exit_finish;
	} else {
		strcpy(gts_test->test_result, "0P");
	}

	/* include rawcap test and deltacap test */
	goodix_capacitance_test(gts_test, info);
	goodix_store_noisedata(gts_test, info);
	goodix_shortcircut_test(gts_test);

	strcpy(info->result, gts_test->test_result);

exit_finish:
	goodix_ts->enter_rawtest = false;
	goodix_tptest_finish(gts_test);
	kfree(gts_test);
	gts_test = NULL;
	return 0;
}

struct goodix_ts_fw_data * goodix_get_fw_data_para(void)
{
	return &goodix_short_fw_data;
}

int goodix_get_fw_data(void)
{
	struct goodix_ts_fw_data *fw_data = NULL;
	const struct firmware *short_fw = NULL;
	int ret = -1;

	fw_data = goodix_get_fw_data_para();

	TS_LOG_INFO("Get shortcircut firmware ...");
	memset(fw_data->short_fw_data, 0, sizeof(fw_data->short_fw_data));
	fw_data->fw_exist = 0;
	fw_data->short_fw_size= 0;
	if (0 == fw_data->fw_exist){
		ret = request_firmware(&short_fw, SHORT_FILE_PATH,
				&goodix_ts->pdev->dev);
		if (ret < 0) {
			TS_LOG_ERR("Request shortcircut firmware failed - %s (%d)", SHORT_FILE_PATH, ret);
			goto end;
		}
		TS_LOG_INFO("Request shortcircut firmware success");
	}
	if (short_fw != NULL){
		if (short_fw->size > GTP_SHORTCIRCUT_TEST_IMAMGE_BUFF_MAX){
			TS_LOG_ERR("Request firmware: short_fw->size > TS_SHORTCIRCUT_TEST_IMAMGE_BUFF_MAX, get firmware fail");
			goto end;
		}
		fw_data->fw_exist = 1;
		memcpy(fw_data->short_fw_data, short_fw->data, GTP_SHORTCIRCUT_TEST_IMAMGE_BUFF_MAX);
		fw_data->short_fw_size= short_fw->size;
	}
	TS_LOG_INFO("ShortCuit Firmware size: %d,dsp_short[0]: 0x%02x dsp_short[1]: 0x%02x dsp_short[2]: 0x%02x",
				fw_data->short_fw_size,fw_data->short_fw_data[0], fw_data->short_fw_data[1], fw_data->short_fw_data[2]);
end:
	if (short_fw != NULL) {
		release_firmware(short_fw);
	}
	return ret;
}

/* get short firmware */
static int goodix_get_short_firmware(struct goodix_ts_fw_data *fw_data)
{
	int ret = -1;

	TS_LOG_INFO("Get shortcircut firmware ...\n");
	if(0 == fw_data->fw_exist){
		TS_LOG_INFO("No shortcircut firmware\n");
		return ret;
	}
	TS_LOG_INFO("Firmware size: %d,dsp_short[0]: 0x%02x dsp_short[1]: 0x%02x dsp_short[2]: 0x%02x\n",
				fw_data->short_fw_size,fw_data->short_fw_data[0], fw_data->short_fw_data[1], fw_data->short_fw_data[2]);
	return 0;
}

int __gt1x_hold_ss51_dsp_20(void)
{
	int ret = -1;
	int retry = 0;
	u8 buf[1];
	int hold_times = 0;

	while (retry++ < 30) {
        
		// Hold ss51 & dsp
		buf[0] = 0x0C;
		ret = goodix_i2c_write(_rRW_MISCTL__SWRST_B0_, buf, 1);
		if (ret) {
			GTP_ERROR("Hold ss51 & dsp I2C error,retry:%d", retry);
			continue;
		}
		// Confirm hold
		buf[0] = 0x00;
		ret = goodix_i2c_read(_rRW_MISCTL__SWRST_B0_, buf, 1);
		if (ret) {
			GTP_ERROR("Hold ss51 & dsp I2C error,retry:%d", retry);
			continue;
		}
		if (0x0C == buf[0]) {
			if (hold_times++ < 20) {
				continue;
			} else {
				break;
			}
		}
		GTP_ERROR("Hold ss51 & dsp confirm 0x4180 failed,value:%d", buf[0]);
	}
	if (retry >= 30) {
		GTP_ERROR("Hold ss51&dsp failed!");
		return -EINVAL;
	}

	GTP_INFO("Hold ss51&dsp successfully.");
	return 0;
}
static int goodix_hold_ss51_dsp(struct goodix_ts_test *ts_test)
{
	int ret = -1;
	u8 buf[1];

	/* chip reset */
	ts_test->ts->ops.chip_reset();

	/* hold dsp ss51 */
	ret = __gt1x_hold_ss51_dsp_20();
	if (ret < 0) {
		return ret;
	}

	/* write 0x5094 */
	buf[0] = 0x00;
	ret = ts_test->ts->ops.i2c_write(_bRW_MISCTL__TMR0_EN, buf, 1);
	if (ret < 0) {
		GTP_ERROR("Write 0x5094 error.");
		return ret;
	}

	/* CPU_SWRST */
	buf[0] = 0x00;
	ret = ts_test->ts->ops.i2c_write(_bRW_MISCTL__CACHE_EN, buf, 1);
	if (ret < 0) {
		GTP_ERROR("CPU_SWRST failed.");
		return ret;
	}

	/* write 0x4190 */
	buf[0] = 0x02;
	ret = ts_test->ts->ops.i2c_write(_rRW_MISCTL__BOOTCTL_B0_, buf, 1);
	if (ret < 0) {
		GTP_ERROR("Write 0x4190 error.");
		return ret;
	}

	/* set scramble */
	buf[0] = 0x00;
	ret = ts_test->ts->ops.i2c_write(_bRW_MISCTL__SRAM_BANK, buf, 1);
	if (ret < 0) {
		GTP_ERROR("set scramble failed.");
		return ret;
	}

	/* clear cache enable */
	buf[0] = 0x01;
	ret = ts_test->ts->ops.i2c_write(_bRW_MISCTL__MEM_CD_EN, buf, 1);
	if (ret < 0) {
		GTP_ERROR("Disable watchdog failed.");
		return ret;
	}
	return ret;
}

static s32 load_code_and_check(unsigned char *codes, int size, struct goodix_ts_test *ts_test)
{
	u8 i, count, packages;
	u8 *ram;
	u16 start_addr, tmp_addr;
	s32 len, code_len, ret = -1;

	ram = (u8 *) kmalloc(PACKAGE_SIZE,GFP_KERNEL);
	if (ram == NULL) {
		return -EINVAL;
	}

	start_addr = 0xC000;
	len = PACKAGE_SIZE;
	tmp_addr = start_addr;
	count = 0;
	code_len = size;
	packages = code_len / PACKAGE_SIZE + 1;

	for (i = 0; i < packages; i++) {
		if (len > code_len) {
			len = code_len;
		}

		ts_test->ts->ops.i2c_write(tmp_addr, (u8 *) & codes[tmp_addr - start_addr], len);
		ts_test->ts->ops.i2c_read(tmp_addr, ram, len);
		ret = memcmp(&codes[tmp_addr - start_addr], ram, len);

		if (ret) {
			if (count++ > 5) {
				GTP_ERROR("equal error.");
				break;
			}
			continue;
		}

		tmp_addr += len;
		code_len -= len;

		if (code_len <= 0) {
			break;
		}
	}

	kfree(ram);
	if (count < 5) {
		GTP_INFO("Burn DSP code successfully!");
		return 1;
	}

	return -1;
}

static int dsp_fw_startup(struct goodix_ts_test *ts_test)
{
	unsigned char buf[8];
	int ret = -1;

	/* disable patch area access */
	buf[0] = 0x00;
	ret = ts_test->ts->ops.i2c_write(_rRW_MISCTL__SHORT_BOOT_FLAG, buf, 1);
	if (ret < 0) {
		GTP_ERROR("disable patch area access fail!");
		return ret;
	}

	buf[0] = 0x03;
	ret = ts_test->ts->ops.i2c_write(_rRW_MISCTL__BOOT_OPT_B0_, buf, 1);
	if (ret < 0) {
		GTP_ERROR("release ss51 fail!");
		return ret;
	}
	buf[0] = 0x01;
	ret = ts_test->ts->ops.i2c_write(_bWO_MISCTL__CPU_SWRST_PULSE, buf, 1);
	if (ret < 0) {
		GTP_ERROR("release ss51 fail!");
		return ret;
	}
	buf[0] = 0x08;
	ret = ts_test->ts->ops.i2c_write(_rRW_MISCTL__SWRST_B0_, buf, 1);
	if (ret < 0) {
		GTP_ERROR("release ss51 fail!");
		return ret;
	}
	buf[0] = 0xA0;
	ret = ts_test->ts->ops.i2c_write(0X4274, buf, 1);
	if (ret < 0) {
		GTP_ERROR("set 0X4274 0xA0 fail!");
		return ret;
	}
	return ret;
}

/* short test */
static int goodix_short_test_prepare(struct goodix_ts_test *ts_test)
{
	u8 sen_data[32]; 
	u8 drv_data[32];
	int ret, retry = 2;
	u16 drv_offest, sen_offest, drv_index, sen_index;
	u8 chksum = 0x00;
	u8 buf[2];
	u32 i;
	struct goodix_ts_fw_data *gts_fw_data = NULL;

	GTP_INFO("Short test prepare+");
	gts_fw_data = goodix_get_fw_data_para();
	ret = goodix_get_short_firmware(gts_fw_data);
	if (ret < 0)
	{
		GTP_ERROR("Request firmware failed, exit short test");
		return ret;
	}

	ret = goodix_hold_ss51_dsp(ts_test);
	if (ret < 0)
	{
		GTP_ERROR("Hold ss51 & dsp failed, exit short test");
		return ret;
	}

	/* preparation of downloading the DSP code */
	ret = load_code_and_check((u8 *)gts_fw_data->short_fw_data, gts_fw_data->short_fw_size, ts_test);
	if (ret < 0)
	{
		GTP_ERROR("Load code failed, exit short test");
		return ret;
	}

	ret = dsp_fw_startup(ts_test);
	if (ret < 0)
	{
		GTP_ERROR("Dsp fw startup failed, exit short test");
		return ret;
	}

	msleep(30);
	buf[0]=0x00;
	while (retry--) {
		/* check firmware running */
		for (i = 0; i < 50; i++) {
			ret =goodix_i2c_read(0x5095, buf, 1);
			printk("0x5095 = %d",buf[0]);
			if (ret < 0)
				return ret;
			else if (buf[0] == 0xaa)
				break;
			msleep(20);
		}

		if (i < 50)
			break;
		else
			ts_test->ts->ops.chip_reset();
	}

	if (retry <= 0) {
		GTP_ERROR("Switch to short test mode timeout");
		return -1;
	}

	GTP_INFO("Firmware in short test mode");
	buf[0] = (ts_test->test_params.short_threshold >> 8) & 0xff;
	buf[1] = ts_test->test_params.short_threshold & 0xff;
	ret = ts_test->ts->ops.i2c_write(0x8804, buf, 2);
	if (ret < 0)
		return ret;

	/* ADC Read Delay */
	buf[0] = 0;
	buf[1] = GTP_ADC_READ_DELAY & 0xff;
	ret = ts_test->ts->ops.i2c_write(0x8806, buf, 2);
	if (ret < 0)
		return ret;

	/* DiffCode Short Threshold */
	buf[0] =0;
	buf[1] = 20 & 0xff;
	ret = ts_test->ts->ops.i2c_write(0x8851,buf,2);
	if (ret < 0)
		return ret;

	memset(sen_data, 0xFF, sizeof(sen_data));
	memset(drv_data, 0xFF, sizeof(drv_data));
	drv_offest = 0x80d7 - 0x8047;
	sen_offest = 0x80b7 - 0x8047;

	for(i = 0; i < ts_test->test_params.sen_num; i++)
		{
		sen_index = sen_offest + i;
		sen_data[i] = ts_test->test_config.data[sen_index];
		}
	for(i = 0; i < ts_test->test_params.drv_num; i++)
		{
		drv_index = drv_offest + i;
		drv_data[i] = ts_test->test_config.data[drv_index];
		}
	for (i = 0; i < sizeof(sen_data); i++)
		chksum += sen_data[i];
	for (i = 0; i < sizeof(drv_data); i++)
		chksum += drv_data[i];
	chksum = 0 - chksum;
	ret = ts_test->ts->ops.i2c_write(0x8808,&drv_data[0], 32);
	if (ret < 0)
		return ret;

	ret = ts_test->ts->ops.i2c_write(0x8832, &sen_data[0], 32);
	if (ret < 0)
		return ret;

	ret = ts_test->ts->ops.i2c_write( 0x8850, &chksum, 1);
	if (ret < 0)
		return ret;

	buf[0] = 0x00;
	ret = ts_test->ts->ops.i2c_write(0x8853, buf, 1);
	if (ret < 0)
		return ret;

	/* clr 5095, runing dsp */
	buf[0] = 0x04;//data[0] = 0x00;
	ret = ts_test->ts->ops.i2c_write(0x5095, buf, 1);
	if (ret < 0)
		return ret;

	GTP_INFO("Short test prepare-");
	return 0;
}

/**
 * goodix_calc_resisitance - calculate resistance
 * @self_capdata: self-capacitance value
 * @adc_signal: ADC signal value
 * return: resistance value
 */
static inline long goodix_calc_resisitance(u16 self_capdata, u16 adc_signal)
{
	return (long)self_capdata * 81 * FLOAT_AMPLIFIER /
				adc_signal - (81 * FLOAT_AMPLIFIER);
}

static int goodix_check_resistance_to_gnd(struct ts_test_params *test_params,
				u16 adc_signal, u8 pos)
{
	int max_drvs = 32;
	long r;
	u16 r_th, avdd_value;

	avdd_value = test_params->avdd_value;
	if (adc_signal == 0 || adc_signal == 0x8000)
		adc_signal |= 1;

	if ((adc_signal & 0x8000) == 0)	/* short to GND */
		r = (long)(5266285) / (adc_signal & (~0x8000)) -
			40 * 100;	//52662.85/code-40
	else	/* short to VDD */
		r = (long)36864 * (avdd_value - 9) *
			100 / ((adc_signal & (~0x8000)) * 7) -
			40 * 100;

	r *= 2;
	r = r / 100;
	if(r > 65535)
		r = 65535;
	else if (r < 0) {
		r = 0;
	}

	if (pos < max_drvs)
		r_th = test_params->r_drv_gnd_threshold;
	else
		r_th = test_params->r_sen_gnd_threshold;

	if (r < r_th) {
		if ((adc_signal & (0x8000)) == 0) {
			if (pos < max_drvs)
				GTP_ERROR("Tx%d shortcircut to GND,R=%ldK,R_Threshold=%dK",
					pos, r, r_th);
			else
				GTP_ERROR("Rx%d shortcircut to GND,R=%ldK,R_Threshold=%dK",
					pos - max_drvs, r, r_th);
		} else {
			if (pos < max_drvs)
				GTP_ERROR("Tx%d shortcircut to VDD,R=%ldK,R_Threshold=%dK",
					pos, r, r_th);
			else
				GTP_ERROR("Rx%d shortcircut to VDD,R=%ldK,R_Threshold=%dK",
					pos - max_drvs, r, r_th);
		}

		return -1;
	}

	return 0;
}

static int goodix_check_short_resistance(u16 self_capdata, u16 adc_signal,
				unsigned short short_r_th)
{
	unsigned short short_val;
	long r;
	int ret = 0;

	if (self_capdata == 0xffff || self_capdata == 0) 
		return 0;

	r = goodix_calc_resisitance(self_capdata, adc_signal);
	r = r / FLOAT_AMPLIFIER;
	if (r > 65535)
		r = 65535;
	short_val = (r >= 0 ? r : 0);

	if (short_val < short_r_th) {
		GTP_ERROR("Short circut:R=%dK,R_Threshold=%dK",
					short_val, short_r_th);
		ret = -1;
	}

	return ret;
}

static int goodix_shortcircut_analysis(struct goodix_ts_test *ts_test)
{
	u8 short_flag, *data_buf = NULL, short_status[3];
	int max_drvs = 32, max_sens  = 32;
	u16 self_capdata[max_drvs + max_sens], short_pin_num;
	u16 adc_signal, data_addr;
	int ret = 0, err = 0;
	u16 r_threshold;
	int size, i, j, self_capdata_offset, adc_signal_offset;

	ret = ts_test->ts->ops.i2c_read(0x8801,  &short_flag, 1);
	if (ret < 0) {
		return ret;
	} else if ((short_flag & 0x0F) == 0x00) {
		GTP_INFO("No shortcircut");
		return 0;
	}

//	data_buf = kzalloc(256, GFP_KERNEL);
	data_buf = kzalloc(1024, GFP_KERNEL);
	if (!data_buf) {
		GTP_ERROR("Failed to alloc memory");
		return -ENOMEM;
	}

	/* shortcircut to gnd */
	if ((short_flag & 0x08) == 0x08) {
		/* read diff code, diff code will be used to calculate
			resistance between channel and GND */
		size = (max_drvs + max_sens) * 2;
		ret = ts_test->ts->ops.i2c_read(0xA531,  data_buf, size);
		if (ret < 0)
			goto exit_kfree;

		for (i = 0; i < size; i += 2) {
			adc_signal = be16_to_cpup((__be16*)&data_buf[i]);
			ret = goodix_check_resistance_to_gnd(&ts_test->test_params,
						adc_signal, i / 2);
			if (ret < 0)
				err |= ret;
		}
	}

	/* read self-capdata+ */
	size = (max_drvs + max_sens) * 2;
	ret = ts_test->ts->ops.i2c_read(0xA4A1, data_buf, size);
	if (ret < 0)
		goto exit_kfree;

	for (i = 0;i < max_drvs + max_sens; i++)
		{
		self_capdata_offset =i * 2;
		self_capdata[i] = be16_to_cpup((__be16*)&data_buf[self_capdata_offset]) & 0x7fff;
		}
	/* read self-capdata- */

	/* read tx tx short number */
	ret = ts_test->ts->ops.i2c_read(0x8802, &short_status[0], 3);
	if (ret < 0)
		goto exit_kfree;
	GTP_INFO("Tx&Tx:%d,Rx&Rx:%d,Tx&Rx:%d",short_status[0],
					short_status[1],short_status[2]);

	/* drv&drv shortcircut check */
	data_addr = 0x8800 + 0x60;
	for (i = 0; i < short_status[0]; i++) {
		size = 4 + max_drvs * 2 + 2;
		ret = ts_test->ts->ops.i2c_read(data_addr, data_buf, size);
		if (ret < 0)
			goto exit_kfree;

		r_threshold = ts_test->test_params.r_drv_drv_threshold;
		short_pin_num = be16_to_cpup((__be16 *)&data_buf[0]);
		if (short_pin_num > max_drvs + max_sens)
			continue;

		for (j = i + 1; j < max_drvs; j++) {
			adc_signal_offset = 4 + j * 2;
			adc_signal = be16_to_cpup((__be16 *)&data_buf[adc_signal_offset]);
			if (adc_signal > ts_test->test_params.short_threshold) {
				ret = goodix_check_short_resistance(
						self_capdata[short_pin_num], adc_signal,
						r_threshold);
				if (ret < 0) {
					err |= ret;
					GTP_ERROR("Tx%d-Tx%d shortcircut", short_pin_num, j);
				}
			}
		}
		data_addr += size;
	}

	/* sen&sen shortcircut check */
	data_addr = 0xA0D2;
	for (i = 0; i < short_status[1]; i++) {
		size = 4 + max_sens * 2 + 2;
		ret = ts_test->ts->ops.i2c_read(data_addr, data_buf, size);
		if (ret < 0)
			goto exit_kfree;

		r_threshold = ts_test->test_params.r_sen_sen_threshold;
		short_pin_num = be16_to_cpup((__be16 *)&data_buf[0]) + max_drvs;
		if (short_pin_num > max_drvs + max_sens)
			continue;

		for (j = 0; j < max_sens; j++) {
			if(j == i || (j < i && (j & 0x01) == 0))
				continue;
			adc_signal_offset = 4 + j * 21;
			adc_signal = be16_to_cpup((__be16 *)&data_buf[adc_signal_offset]);
			if (adc_signal > ts_test->test_params.short_threshold) {
				ret = goodix_check_short_resistance(
						self_capdata[short_pin_num], adc_signal,
						r_threshold);
				if (ret < 0) {
					err |= ret;
					GTP_ERROR("Rx%d-Rx%d shortcircut",
							short_pin_num - max_drvs, j);
				}
			}
		}
		data_addr += size;
	}

	/* sen&drv shortcircut check */
	data_addr = 0x99e0;
	for (i = 0; i < short_status[2]; i++) {
		size = 4 + max_sens * 2 + 2;
		ret = ts_test->ts->ops.i2c_read(data_addr, data_buf, size);
		if (ret < 0)
			goto exit_kfree;

		r_threshold = ts_test->test_params.r_drv_sen_threshold;
		short_pin_num = be16_to_cpup((__be16 *)&data_buf[0]) + max_drvs;
		if (short_pin_num > max_drvs + max_sens)
			continue;

		for (j = 0; j < max_drvs; j++) {
			adc_signal_offset = 4 + j * 21;
			adc_signal = be16_to_cpup((__be16 *)&data_buf[adc_signal_offset]);
			if (adc_signal > ts_test->test_params.short_threshold) {
				ret = goodix_check_short_resistance(
						self_capdata[short_pin_num], adc_signal,
						r_threshold);
				if (ret < 0) {
					err |= ret;
					GTP_ERROR("Rx%d-Tx%d shortcircut",
							short_pin_num - max_drvs, j);
				}
			}
		}
		data_addr += size;
	}

exit_kfree:
	kfree(data_buf);
	return err | ret ? -EFAULT : 0;
}

static int goodix_shortcircut_test(struct goodix_ts_test *ts_test)
{
	u16 reg_sta = 0x8800;
	u8 data[2];
	int ret, retry = GTP_SHORTCIRCUT_TEST_RETRY;

	ret = goodix_short_test_prepare(ts_test);
	if (ret < 0)
		goto end_test;

	while (retry--) {
		ret = ts_test->ts->ops.i2c_read(reg_sta, data, 1);
		if (ret < 0)
			goto end_test;
		else if (data[0] == 0x88)
			break;

		msleep(50);
		GTP_DEBUG("waitting...:%d", retry);
	}

	if (retry <= 0) {
		GTP_ERROR("Wait short test finish timeout");
		ret = -1;
		goto end_test;
	}

	ret = goodix_shortcircut_analysis(ts_test);
	if (ret < 0) {
		GTP_ERROR("Shortcircut test failed");
		goto end_test;
	}

end_test:
	if (ret < 0)
		strcat(ts_test->test_result, "-3F");
	else
		strcat(ts_test->test_result, "-3P");

	return ret;
}

