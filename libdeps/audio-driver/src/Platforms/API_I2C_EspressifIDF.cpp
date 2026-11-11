// I2C Driver for Espressif IDF
#if !defined(ARDUINO) || AUDIO_DRIVER_FORCE_IDF

#include <assert.h>
#include <string.h>  // memcpy

#include "Platforms/API_I2C.h"
#include "driver/i2c_master.h"

// store a collection of configs by port number
static I2CConfig *i2c_configs[5];

I2CConfig *get_config(i2c_bus_handle_t hdl) {
  for (int j = 0; j < 5; j++) {
    if (i2c_configs[j]->p_wire == hdl) {
      return i2c_configs[j];
    }
  }
  return nullptr;
}

error_t i2c_bus_create(struct I2CConfig *config) {
  I2CConfig &pins = *config;

  i2c_port_t i2c_slave_port = (i2c_port_t)pins.port;
  i2c_configs[pins.port] = config;

  AD_LOGI("i2c_bus_create port: %d scl: %d, sda: %d address: 0x%x", pins.port,
          pins.scl, pins.sda, pins.address);

  i2c_master_bus_config_t i2c_mst_config = {};
  i2c_mst_config.clk_source = I2C_CLK_SRC_DEFAULT;
  i2c_mst_config.i2c_port = i2c_slave_port;
  i2c_mst_config.scl_io_num = (gpio_num_t)pins.scl;
  i2c_mst_config.sda_io_num = (gpio_num_t)pins.sda;
  i2c_mst_config.glitch_ignore_cnt = 7;
  i2c_mst_config.flags.enable_internal_pullup = true;

  i2c_master_bus_handle_t bus_handle;
  if (i2c_new_master_bus(&i2c_mst_config, &bus_handle) != ESP_OK) {
    AD_LOGE("i2c_new_master_bus");
    return ESP_FAIL;
  }

  if (i2c_master_probe(bus_handle, pins.address, -1) != RESULT_OK) {
    AD_LOGE("Address check failed: scanning addresses:");
    for (int j = 0; j < 127; j++) {
      auto rc = i2c_master_probe(bus_handle, j, -1);
      AD_LOGE("- address: 0x%x -> %d", j, rc);
    }
    return ESP_FAIL;
  }

  // store dev_handle
  pins.p_wire = bus_handle;

  return ESP_OK;
}

void i2c_bus_delete(i2c_bus_handle_t bus) {
  if (i2c_del_master_bus((i2c_master_bus_handle_t)bus) != ESP_OK) {
    AD_LOGI("i2c_bus_delete");
  }
}

error_t i2c_bus_write_bytes(i2c_bus_handle_t bus, int addr, uint8_t *reg,
                            int reglen, uint8_t *data, int datalen) {
  AD_LOGD("i2c_bus_write_bytes address: 0x%x", addr);
  i2c_master_bus_handle_t bus_handle = (i2c_master_bus_handle_t)bus;

  I2CConfig *cfg = get_config(bus);
  assert(cfg != nullptr);
  assert(cfg->address == addr);

  i2c_device_config_t dev_cfg = {};
  dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  dev_cfg.device_address = cfg->address;
  dev_cfg.scl_speed_hz = cfg->frequency;

  i2c_master_dev_handle_t dev_handle;
  if (!i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle) == ESP_OK) {
    AD_LOGE("i2c_new_master_bus");
    return ESP_FAIL;
  }

  // assemble data to send
  int total_len = reglen + datalen;
  uint8_t total_data[total_len];
  memcpy(total_data, reg, reglen);
  memcpy(total_data + reglen, data, datalen);

  esp_err_t ret = ESP_OK;
  ret |= i2c_master_transmit(dev_handle, total_data, total_len, -1);
  if (ret == ESP_OK) {
    ret = i2c_master_bus_wait_all_done(bus_handle, -1);
    if (ret != ESP_OK) AD_LOGE("i2c_master_bus_wait_all_done");
  } else {
    AD_LOGE("i2c_master_transmit");
  }

  if (i2c_master_bus_rm_device(dev_handle) != ESP_OK) {
    AD_LOGE("i2c_master_bus_rm_device");
  }

  if (ret != ESP_OK) {
    AD_LOGE("i2c_bus_write_bytes");
  }
  return ret;
}

error_t i2c_bus_read_bytes(i2c_bus_handle_t bus, int addr, uint8_t *reg,
                           int reglen, uint8_t *read_buffer, int read_size) {
  // get port
  AD_LOGD("i2c_bus_read_bytes address: 0x%x", addr);
  i2c_master_bus_handle_t bus_handle = (i2c_master_bus_handle_t)bus;

  I2CConfig *cfg = get_config(bus);
  assert(cfg != nullptr);
  assert(cfg->address == addr);

  i2c_device_config_t dev_cfg = {};
  dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  dev_cfg.device_address = cfg->address;
  dev_cfg.scl_speed_hz = cfg->frequency;

  i2c_master_dev_handle_t dev_handle;
  if (!i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle) == ESP_OK) {
    AD_LOGE("i2c_master_bus_add_device");
    return ESP_FAIL;
  }

  // assemble data to send
  int write_size = reglen;
  uint8_t write_buffer[write_size];
  memcpy(write_buffer + 1, reg, reglen);

  esp_err_t ret = ESP_OK;
  ret = i2c_master_transmit_receive(dev_handle, write_buffer, write_size,
                                    read_buffer, read_size, -1);
  ret = i2c_master_bus_wait_all_done(bus_handle, -1);
  if (ret != ESP_OK) {
    AD_LOGE("i2c_bus_read_bytes");
  }
  if (i2c_master_bus_rm_device(dev_handle) != ESP_OK) {
    AD_LOGI("i2c_master_bus_rm_device");
  }

  return ret;
}

#endif