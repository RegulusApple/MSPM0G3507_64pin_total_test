"""Static regression check for the external OLED wiring on MSP-LITO-G3507."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def require(text: str, fragment: str, message: str) -> None:
    if fragment not in text:
        raise AssertionError(message)


def main() -> None:
    soft_i2c = (ROOT / "usr" / "src" / "soft_i2c.c").read_text(
        encoding="utf-8-sig"
    )
    syscfg = (ROOT / "mspm0_g3507.syscfg").read_text(encoding="utf-8-sig")
    generated = (ROOT / "ti_msp_dl_config.h").read_text(encoding="utf-8-sig")
    application = (ROOT / "main.c").read_text(encoding="utf-8-sig")

    require(
        soft_i2c,
        "#define SOFT_I2C_PORT                 I2C_LCD_PINS_PORT",
        "OLED driver must use the SysConfig-generated port macro",
    )
    require(
        soft_i2c,
        "#define SOFT_I2C_SDA_PIN              I2C_LCD_PINS_LCD_SDA_PIN",
        "OLED driver must use the SysConfig-generated SDA macro",
    )
    require(
        soft_i2c,
        "#define SOFT_I2C_SDA_IOMUX            I2C_LCD_PINS_LCD_SDA_IOMUX",
        "OLED driver must use the SysConfig-generated SDA IOMUX macro",
    )
    require(
        soft_i2c,
        "#define SOFT_I2C_SCL_PIN              I2C_LCD_PINS_LCD_SCL_PIN",
        "OLED driver must use the SysConfig-generated SCL macro",
    )
    require(
        soft_i2c,
        "#define SOFT_I2C_SCL_IOMUX            I2C_LCD_PINS_LCD_SCL_IOMUX",
        "OLED driver must use the SysConfig-generated SCL IOMUX macro",
    )
    require(
        syscfg,
        'GPIO3.associatedPins[0].pin.$assign              = "PA0";',
        "SysConfig must reserve PA0 for OLED SDA",
    )
    require(
        syscfg,
        'GPIO3.associatedPins[1].pin.$assign              = "PA1";',
        "SysConfig must reserve PA1 for OLED SCL",
    )
    require(
        generated,
        "#define I2C_LCD_PINS_LCD_SDA_PIN                                 (DL_GPIO_PIN_0)",
        "Generated OLED SDA must resolve to PA0 (J2 pin 23)",
    )
    require(
        generated,
        "#define I2C_LCD_PINS_LCD_SCL_PIN                                 (DL_GPIO_PIN_1)",
        "Generated OLED SCL must resolve to PA1 (J2 pin 24)",
    )
    require(
        application,
        "DL_GPIO_togglePins(KEY_LED_PORT, KEY_LED_LED_PIN)",
        "Status indication must use the SysConfig-generated PB14 LED macros",
    )

    print("EXTERNAL_OLED_PIN_TEST,PASS,BOARD=MSP-LITO,SDA=PA0,SCL=PA1")


if __name__ == "__main__":
    main()
