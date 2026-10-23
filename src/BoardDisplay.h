#pragma once
#include <LovyanGFX.hpp>
class BoardDisplay : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 panel;
  lgfx::Bus_SPI bus;
  lgfx::Light_PWM backlight;
public:
  BoardDisplay() {
    auto b=bus.config(); b.spi_host=SPI2_HOST; b.spi_mode=0; b.freq_write=40000000;
    b.pin_sclk=40; b.pin_mosi=45; b.pin_miso=-1; b.pin_dc=41;
    b.spi_3wire=true; b.use_lock=true; b.dma_channel=SPI_DMA_CH_AUTO;
    bus.config(b); panel.setBus(&bus);
    auto p=panel.config(); p.pin_cs=42; p.pin_rst=39; p.pin_busy=-1;
    p.memory_width=240; p.memory_height=320; p.panel_width=172; p.panel_height=320;
    p.offset_x=34; p.offset_y=0; p.invert=true; p.rgb_order=false;
    p.readable=false; p.bus_shared=false; panel.config(p);
    auto l=backlight.config(); l.pin_bl=46; l.invert=false; l.freq=12000; l.pwm_channel=7;
    backlight.config(l); panel.setLight(&backlight); setPanel(&panel);
  }
};
