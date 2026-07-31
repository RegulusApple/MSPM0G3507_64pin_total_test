#include "usr_uart.h"

#include <stdarg.h>

#define USR_UART_TX_TIMEOUT_CYCLES (CPUCLK_FREQ / 10U)

static volatile uint8_t gRxBuffer[USR_UART_RX_BUFFER_SIZE];
static volatile uint16_t gRxWriteIndex;
static volatile uint16_t gRxReadIndex;
static volatile bool gRxOverflow;

static char gLineBuffer[64];
static char gCompletedLine[64];
static uint16_t gLineLength;
static volatile bool gLineReady;
static volatile bool gLineOverflow;
static bool gDiscardLine;

static int USR_UART_printUnsigned(uint32_t value, uint32_t base, bool uppercase)
{
    char digits[16];
    int count = 0;
    int written = 0;

    if (value == 0U) {
        USR_UART_sendByte('0');
        return 1;
    }

    while ((value > 0U) && (count < (int) sizeof(digits))) {
        uint32_t digit = value % base;
        if (digit < 10U) {
            digits[count++] = (char) ('0' + digit);
        } else if (uppercase) {
            digits[count++] = (char) ('A' + (digit - 10U));
        } else {
            digits[count++] = (char) ('a' + (digit - 10U));
        }
        value /= base;
    }

    while (count > 0) {
        USR_UART_sendByte((uint8_t) digits[--count]);
        written++;
    }

    return written;
}

static int USR_UART_printUnsignedWidth(uint32_t value, uint32_t base, bool uppercase,
    uint8_t minWidth, bool zeroPad)
{
    char digits[16];
    int count = 0;
    int written = 0;
    char padChar = zeroPad ? '0' : ' ';

    if (value == 0U) {
        digits[count++] = '0';
    } else {
        while ((value > 0U) && (count < (int) sizeof(digits))) {
            uint32_t digit = value % base;
            if (digit < 10U) {
                digits[count++] = (char) ('0' + digit);
            } else if (uppercase) {
                digits[count++] = (char) ('A' + (digit - 10U));
            } else {
                digits[count++] = (char) ('a' + (digit - 10U));
            }
            value /= base;
        }
    }

    while ((uint8_t) count < minWidth) {
        USR_UART_sendByte((uint8_t) padChar);
        written++;
        minWidth--;
    }

    while (count > 0) {
        USR_UART_sendByte((uint8_t) digits[--count]);
        written++;
    }

    return written;
}

static int USR_UART_printSigned(int32_t value)
{
    if (value < 0) {
        USR_UART_sendByte('-');
        return 1 + USR_UART_printUnsigned((uint32_t) (-value), 10U, false);
    }

    return USR_UART_printUnsigned((uint32_t) value, 10U, false);
}

static uint32_t USR_UART_pow10(uint8_t precision)
{
    uint32_t value = 1U;

    while (precision > 0U) {
        value *= 10U;
        precision--;
    }

    return value;
}

static int USR_UART_printFloat(double value, uint8_t precision)
{
    int written = 0;
    uint32_t scale;
    uint32_t integerPart;
    uint32_t fractionPart;
    uint32_t i;

    /* 闂傚嫭鍔曢崺妤冧焊韫囨梹娈跺ù锝呯▌缁辨繃绌卞┑濠勬 10^precision 闁告瑯鍨禍鎺斺偓鐟邦槸閸欏繘寮ㄩ幆褍寮� uint32_t闁靛棴鎷�
     * 6 濞达絽绉撮惃顒勫极閺夊灝鍤掔紓浣哥箺閸愮粯寰勯悢鐑樻殢濞存粌娴烽弫鎼佸储鐎ｃ劉鍋撴笟鈧。鍫曟偝閸モ晝鎼奸悹瀣暢閻︻垱娼忛幘鍐叉瘔闁靛棴鎷�
     */
    if (precision > 6U) {
        precision = 6U;
    }

    if (value < 0.0) {
        USR_UART_sendByte('-');
        written++;
        value = -value;
    }

    scale = USR_UART_pow10(precision);
    value += 0.5 / (double) scale;

    integerPart  = (uint32_t) value;
    fractionPart = (uint32_t) ((value - (double) integerPart) * (double) scale);

    written += USR_UART_printUnsigned(integerPart, 10U, false);

    if (precision == 0U) {
        return written;
    }

    USR_UART_sendByte('.');
    written++;

    for (i = scale / 10U; i > 0U; i /= 10U) {
        USR_UART_sendByte((uint8_t) ('0' + ((fractionPart / i) % 10U)));
        written++;
    }

    return written;
}

static void USR_UART_storeRxByte(uint8_t data)
{
    uint16_t nextWriteIndex = (uint16_t) ((gRxWriteIndex + 1U) % USR_UART_RX_BUFFER_SIZE);

    if (nextWriteIndex == gRxReadIndex) {
        gRxOverflow = true;
        return;
    }

    gRxBuffer[gRxWriteIndex] = data;
    gRxWriteIndex           = nextWriteIndex;
}

static void USR_UART_storeLine(void)
{
    uint16_t i;

    if ((gLineLength == 0U) || gLineReady) {
        gLineLength = 0U;
        return;
    }

    for (i = 0U; i < gLineLength; i++) {
        gCompletedLine[i] = gLineBuffer[i];
    }
    gCompletedLine[gLineLength] = '\0';
    gLineReady = true;

    gLineLength = 0U;
}

static void USR_UART_waitTxDone(void)
{
    uint32_t timeout = USR_UART_TX_TIMEOUT_CYCLES;

    while ((DL_UART_Main_isBusy(UART_0_INST) == true) && (timeout > 0U)) {
        timeout--;
    }
}

void USR_UART_init(void)
{
    gRxWriteIndex = 0U;
    gRxReadIndex  = 0U;
    gRxOverflow   = false;
    gLineLength   = 0U;
    gLineReady    = false;
    gLineOverflow = false;
    gDiscardLine  = false;

    DL_UART_Main_clearInterruptStatus(UART_0_INST,
        DL_UART_MAIN_INTERRUPT_RX | DL_UART_MAIN_INTERRUPT_RX_TIMEOUT_ERROR |
            DL_UART_MAIN_INTERRUPT_OVERRUN_ERROR | DL_UART_MAIN_INTERRUPT_BREAK_ERROR |
            DL_UART_MAIN_INTERRUPT_PARITY_ERROR | DL_UART_MAIN_INTERRUPT_FRAMING_ERROR);
}

void USR_UART_sendByte(uint8_t data)
{
    DL_UART_Main_transmitDataBlocking(UART_0_INST, data);
}

void USR_UART_sendBytes(const uint8_t *data, uint16_t length)
{
    uint16_t i;

    if (data == 0) {
        return;
    }

    for (i = 0U; i < length; i++) {
        USR_UART_sendByte(data[i]);
        USR_UART_waitTxDone();
    }
}

void USR_UART_sendString(const char *str)
{
    if (str == 0) {
        return;
    }

    while (*str != '\0') {
        USR_UART_sendByte((uint8_t) *str);
        str++;
    }
}

int USR_UART_printf(const char *format, ...)
{
    int written = 0;
    va_list args;

    if (format == 0) {
        return 0;
    }

    /* 闁哄牜鍓濊闁哄鍔曞▍鎺楀矗椤忓懏鏆滈柟闀愮娴兼劗绮欑€ｂ晞鍘悽顖氭憸閺併倝鎯冮崟顒傚鐎殿喖绻堥埀顒婃嫹
     * 濞戞挸绉烽惃鐔兼偨閵婏妇鍨奸柛鎴嫹 printf/vsnprintf闁挎稑鐭傛导鈺呭礂瀹ュ懐绌块柛蹇嬪劥缁诲秶鎮扮仦鐣屾皑閻庝絻澹堥崵褎绋夌€ｎ厽绁伴柟瀛樼墳閻ㄧ喓鎷犻弴姘辩憹缂佸鍟块悾楣冨Υ閿燂拷
     */
    va_start(args, format);

    while (*format != '\0') {
        uint8_t precision = 6U;
        uint8_t minWidth = 0U;
        bool zeroPad = false;
        bool longModifier = false;

        if (*format != '%') {
            USR_UART_sendByte((uint8_t) *format++);
            written++;
            continue;
        }

        format++;
        if (*format == '0') {
            zeroPad = true;
            format++;
        }
        while ((*format >= '0') && (*format <= '9')) {
            minWidth = (uint8_t) ((minWidth * 10U) + (uint8_t) (*format - '0'));
            format++;
        }
        if (*format == '.') {
            precision = 0U;
            format++;
            while ((*format >= '0') && (*format <= '9')) {
                precision = (uint8_t) ((precision * 10U) + (uint8_t) (*format - '0'));
                format++;
            }
        }
        if (*format == 'l') {
            longModifier = true;
            format++;
        }
        switch (*format) {
            case 'd':
            case 'i':
                written += USR_UART_printSigned(va_arg(args, int));
                break;
            case 'u':
                written += USR_UART_printUnsignedWidth(va_arg(args, unsigned int), 10U, false,
                    minWidth, zeroPad);
                break;
            case 'x':
                written += USR_UART_printUnsignedWidth(va_arg(args, unsigned int), 16U, false,
                    minWidth, zeroPad);
                break;
            case 'X':
                written += USR_UART_printUnsignedWidth(va_arg(args, unsigned int), 16U, true,
                    minWidth, zeroPad);
                break;
            case 'f':
                (void) longModifier;
                written += USR_UART_printFloat(va_arg(args, double), precision);
                break;
            case 'c':
                USR_UART_sendByte((uint8_t) va_arg(args, int));
                written++;
                break;
            case 's': {
                const char *text = va_arg(args, const char *);
                if (text == 0) {
                    text = "(null)";
                }
                while (*text != '\0') {
                    USR_UART_sendByte((uint8_t) *text++);
                    written++;
                }
                break;
            }
            case '%':
                USR_UART_sendByte('%');
                written++;
                break;
            case '\0':
                format--;
                break;
            default:
                USR_UART_sendByte('%');
                USR_UART_sendByte((uint8_t) *format);
                written += 2;
                break;
        }

        if (*format != '\0') {
            format++;
        }
    }

    va_end(args);
    return written;
}

bool USR_UART_readByte(uint8_t *data)
{
    if ((data == 0) || (gRxReadIndex == gRxWriteIndex)) {
        return false;
    }

    *data        = gRxBuffer[gRxReadIndex];
    gRxReadIndex = (uint16_t) ((gRxReadIndex + 1U) % USR_UART_RX_BUFFER_SIZE);

    return true;
}

bool USR_UART_readLine(char *line, uint16_t maxLength)
{
    uint16_t i;

    if ((line == 0) || (maxLength == 0U) || (gLineReady == false)) {
        return false;
    }

    for (i = 0U; (i < (maxLength - 1U)) && (gCompletedLine[i] != '\0'); i++) {
        line[i] = gCompletedLine[i];
    }
    line[i] = '\0';
    gLineReady = false;

    return true;
}

void USR_UART_process(void)
{
    uint8_t data;

    while (DL_UART_Main_isRXFIFOEmpty(UART_0_INST) == false) {
        data = DL_UART_Main_receiveData(UART_0_INST);
        USR_UART_storeRxByte(data);
    }

    if (gRxOverflow) {
        gRxOverflow = false;
        USR_UART_sendString("ERR:RX_OVERFLOW\r\n");
    }
    if (gLineOverflow) {
        gLineOverflow = false;
        USR_UART_sendString("ERR:LINE_TOO_LONG\r\n");
    }

    while (USR_UART_readByte(&data)) {
        if (data == '\r') {
            continue;
        }

        if (data == '\n') {
            if (gDiscardLine) {
                gDiscardLine = false;
                gLineLength  = 0U;
            } else {
                USR_UART_storeLine();
            }
            continue;
        }

        if (gDiscardLine) {
            continue;
        }

        if (gLineLength < (sizeof(gLineBuffer) - 1U)) {
            gLineBuffer[gLineLength++] = (char) data;
        } else {
            gLineLength = 0U;
            gDiscardLine = true;
            gLineOverflow = true;
        }
    }
}

void USR_UART_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_0_INST)) {
        case DL_UART_MAIN_IIDX_RX:
        case DL_UART_MAIN_IIDX_RX_TIMEOUT_ERROR:
            while (DL_UART_Main_isRXFIFOEmpty(UART_0_INST) == false) {
                USR_UART_storeRxByte(DL_UART_Main_receiveData(UART_0_INST));
            }
            break;
        case DL_UART_MAIN_IIDX_OVERRUN_ERROR:
        case DL_UART_MAIN_IIDX_BREAK_ERROR:
        case DL_UART_MAIN_IIDX_PARITY_ERROR:
        case DL_UART_MAIN_IIDX_FRAMING_ERROR:
            DL_UART_Main_clearInterruptStatus(UART_0_INST,
                DL_UART_MAIN_INTERRUPT_OVERRUN_ERROR | DL_UART_MAIN_INTERRUPT_BREAK_ERROR |
                    DL_UART_MAIN_INTERRUPT_PARITY_ERROR | DL_UART_MAIN_INTERRUPT_FRAMING_ERROR);
            break;
        default:
            break;
    }
}

void UART_0_INST_IRQHandler(void)
{
    USR_UART_IRQHandler();
}
