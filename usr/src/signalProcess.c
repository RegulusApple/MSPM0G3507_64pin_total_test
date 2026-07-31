///////////////////////////////////////////////////////////////////////
//                          信号处理代码
///////////////////////////////////////////////////////////////////////

#include "signalProcess.h"

///////////////////////////////////////////////////////////////////////
//                               FFT
///////////////////////////////////////////////////////////////////////

// 创建正弦查找表
float SIN_TAB[(FFT_LEN >> 2) + 1];

void createSinTab(float *sin_t)
{
    uint16_t i;
    for (i = 0; i <= (FFT_LEN >> 2); i++)
        sin_t[i] = sin(2 * PI * i / FFT_LEN);
}

// 获取正余弦值
float getSinTab(float rad)
{
    int16_t n;
    float a = 0;
    n = (int16_t)(rad * (FFT_LEN >> 1) / PI);    // n = rad / (2*pi/FFT_LEN)
    if (n >= 0 && n <= (FFT_LEN >> 2))
        a = SIN_TAB[n];
    else if (n > (FFT_LEN >> 2) && n < (FFT_LEN >> 1)) {
        n -= (FFT_LEN >> 2);
        a = SIN_TAB[(FFT_LEN >> 2) - n];
    }
    else if (n >= (FFT_LEN >> 1) && n < 3 * (FFT_LEN >> 2)) {
        n -= (FFT_LEN >> 1);
        a = -SIN_TAB[n];
    }
    else if (n >= 3 * (FFT_LEN >> 2) && n < 3 * FFT_LEN) {
        n = FFT_LEN - n;
        a = -SIN_TAB[n];
    }
    return a;
}

float getCosTab(float rad)
{
    float a, rad2;
    rad2 = rad + PI / 2;    // cos(theta) = sin(theta + pi/2)
    if (rad2 > 2 * PI) rad2 -= 2 * PI;
    a = getSinTab(rad2);
    return a;
}

// 复数乘法
void compxMcl(float ar, float ai, float br, float bi, float *cr, float *ci)
{
    // 计算方法：(ar + jai)(br + jbi) = ar*br - ai*bi + j(ar*bi + ai*br)
    // 计 算 量： 4 次实数乘法, 2 次实数加法
    *cr = ar*br - ai*bi;
    *ci = ar*bi + ai*br;
}

/**********************************************************************
 * FFT 运算, 整数 FFT
 * 输入：xin 复数原始数据, 对于 ADC 采样, 输入数据为 {reg[0], 0, reg[1], 0, ...}
 */
void SP_myFFT_f32(float *xin)
{
    // 参考资料：数字信号处理教程 - 程佩青（第五版）
    int16_t nv2, nm1;                       // 变址运算变量
    int16_t i, j, k, l, m, len;             // 临时变量
    int16_t dis, dis0, ip;                  // dis 当前级蝶形结距离；dis0 同一蝶形结两点距离；ip 蝶形结下方节点
    float Wr, Wi, wnr, wni, tmpr, tmpi;     // 复数运算变量

    /* 变址运算：自然序 -> 倒位序（雷德算法） */
    nv2 = (FFT_LEN >> 1);
    nm1 = FFT_LEN - 1;
    j = 0;
    for (i = 0; i < nm1; i++)
    {
        if (i < j)          // 如果 i<j,即进行变址
        {
            tmpr = xin[j<<1];
            xin[j<<1] = xin[i<<1];
            xin[i<<1] = tmpr;
            tmpi = xin[(j<<1)+1];
            xin[(j<<1)+1] = xin[(i<<1)+1];
            xin[(i<<1)+1] = tmpi;
        }
        k = nv2;            // 求j的下一个倒位序
        while (k <= j)      // 如果k<=j,表示j的最高位为1
        {
            j = j - k;      // 把最高位变成0
            k = (k >> 1);   // k/2，比较次高位，依次类推，逐个比较，直到某个位为0
        }
        j = j + k;          // 把0改为1
    }

    /* DIT (Decimation In Time) - FFT  基-2按时间抽选的 DIT - FFT 算法（库利-图基算法）*/
    // 把序列每次一分为二，最后分割成两点 DFT。程序从 2 点 DFT 开始递推
    len = FFT_LEN;
    for (l = 1; (len = (len >> 1)) != 1; l++);            // 计算 l 的值，即蝶形级数，相当于 l=log2(N)
    for (m = 1; m <= l; m++)                        // m 表示第 m 级蝶形，l 为蝶形级总数
    {
        dis = 2 << (m - 1);                         // 蝶形结距离，即第 m 级 W 系数相同的蝶形结相距 dis 个下标
        dis0 = (dis >> 1);                          // 同一蝶形结中参加运算的两点的下标距离
        Wr = 1;                                     // 蝶形结运算系数(W 算子)，每级初始值为 1
        Wi  = 0;                                    // 即 W_N^0 = cos(0) + jsin(0) = 1 + j0 = 1
        wnr = getCosTab(PI / dis0);                 //  wn 为系数商，第 m 级别蝶形运算系数的倍增量
        wni = - getSinTab(PI / dis0);               // 即 W_(N/dis) = cos(-2pi/dis) + jsin(-2pi/dis)
        for (j = 0; j < dis0; j++)                  // 计算系数 W 不同的蝶形结
        {
            for (i = j; i < FFT_LEN; i = i + dis)   // 同一蝶形结运算，即计算系数相同蝶形结
            {
                ip = i + dis0;                      // i，ip分别表示参加蝶形运算的两个节点
                compxMcl(xin[ip << 1], xin[(ip << 1) + 1], Wr, Wi, &tmpr, &tmpi);     // 在一个蝶形结内进行蝶形运算
                xin[ip<<1] = xin[i<<1] - tmpr;
                xin[(ip<<1) + 1] = xin[(i<<1) + 1] - tmpi;
                xin[i<<1] = xin[i<<1] + tmpr;
                xin[(i<<1) + 1] = xin[(i<<1) + 1] + tmpi;
            }
            compxMcl(Wr, Wi, wnr, wni, &Wr, &Wi);
        }
    }
}

/**********************************************************************
 * FFT 初始化（创建三角查找表）
 */
void SP_myFFTInit(void) {
    // 创建查找表
    createSinTab(SIN_TAB);
}

/**********************************************************************
 * FFT
 * 输入：regBuff [寄存器数组], pFFTIOData[FFT输入/输出, 同址运算]
 * 输出：无
 */
void SP_myFFT(uint16_t *regBuff, float *pFFTIOData)
{
    uint16_t i = 0;
    // pFFTPreData -> {real[0], imag[0], real[1], imag[1], ...}
    for(i = 0; i < FFT_LEN; i++)
    {
        // 实部
        pFFTIOData[i<<1] = regBuff[i];
        // 虚部
        pFFTIOData[(i<<1) + 1] = 0;
    }

    // FFT
    SP_myFFT_f32(pFFTIOData);
}

/**********************************************************************
 * 求 FFT 幅度(电压[有效值]!!!)和辐角
 * 输入：pFFTResIn [FFT 输入数据] pFFTFreqPointSelect[FFT 频点选择索引], numPointSelect[点数], adcVref[ADC 参考], pFFTMagOut [FFT 幅度输出], pFFTAngleOut[FFT 辐角输出]
 * 输出：无
 */
void SP_myFFT_CalcMagAngle(float* pFFTResIn, uint16_t* pFFTFreqPointSelect, uint16_t numPointSelect, float adcVref, float* pFFTMagOut, float* pFFTAngleOut)
{
    uint16_t i = 0;
    for(i = 0; i < numPointSelect; i++)
    {
        // FFT 幅度计算
        if (pFFTFreqPointSelect[i] != 0) {
            // 非 0 频点
            pFFTMagOut[i] = sqrt(pFFTResIn[(pFFTFreqPointSelect[i] << 1)] * pFFTResIn[(pFFTFreqPointSelect[i] << 1)] + \
                                 pFFTResIn[(pFFTFreqPointSelect[i] << 1) + 1] * pFFTResIn[(pFFTFreqPointSelect[i] << 1) + 1]) / (FFT_LEN >> 1) * adcVref / 4096 / SQRT2;
        }
        else {
            // 0 频点
            pFFTMagOut[i] = sqrt(pFFTResIn[0] * pFFTResIn[0] + pFFTResIn[1] * pFFTResIn[1]) / FFT_LEN  * adcVref / 4096;
        }
        // FFT 角度计算
        pFFTAngleOut[i] = atan2(pFFTResIn[(pFFTFreqPointSelect[i] << 1) + 1], pFFTResIn[(pFFTFreqPointSelect[i] << 1)]) * 180.0f / PI;
    }
}

/**********************************************************************
 * ADC 采样通道, 相位补偿
 * 输入：pFFTResIn [FFT 输入数据] pFFTFreqPointSelect[FFT 频点选择索引], numPointSelect[点数], adcVref[ADC 参考], pFFTMagOut [FFT 幅度输出], pFFTAngleOut[FFT 辐角输出]
 * 输出：无
 */
void SP_AngleCompensate50HzHarmonic(float* pFFTAngleIn, float* pFFTAngleOut, float compensateDelayS, uint16_t numHarmonic)
{
    uint16_t i = 0;
    float compDegree = 0;
    // 在每个频率分量上, 对通道时延引起的角度偏差进行补偿
    for(i = 0; i < numHarmonic; i++) {
        compDegree = compensateDelayS * 50.0f * (i + 1) * 360;
        pFFTAngleOut[i] = pFFTAngleIn[i] - compDegree;
    }
}

///////////////////////////////////////////////////////////////////////
//                          功率参数分析
///////////////////////////////////////////////////////////////////////

/**********************************************************************
 * 计算 sqrt(M1^2 + M2^2 + M3^2, ...)
 * 输入：pMagInput [幅度表], num [幅度数量]
 * 输出：无
 */
float SP_CalcSumSqrt(float* pMagInput, uint16_t num)
{
    float tmpFlt = 0;
    uint16_t i = 0;
    for (i = 0; i < num; i++)
        tmpFlt = tmpFlt + pMagInput[i] * pMagInput[i];

    return sqrt(tmpFlt);
}

/**********************************************************************
 * 计算 THD
 * 输入：pMagBaseHarmonic [基波, 谐波幅度], numHarmonic [谐波次数], pOutTHD [输出 THD 值]
 * 输出：THD
 */
float SP_CalcTHD(float* pMagBaseHarmonic, uint16_t numHarmonic)
{
    // THD = sqrt(U1^2 + U2^2 +, ...) / U0
    return SP_CalcSumSqrt(&pMagBaseHarmonic[1], numHarmonic - 1) / pMagBaseHarmonic[0];
}

/**********************************************************************
 * 计算电流有效值, 电压有效值, 视在功率 (转换到 220V 电网需要乘上增益系数)
 * 输入：pCurrentMagBaseHarmonic [电流基波, 谐波幅度], pVoltageMagBaseHarmonic [电压基波, 谐波幅度], numHarmonic [谐波次数], currentRms[电流有效值], voltageRms[电压有效值], powerS[视在功率]
 * 输出：无
 */
void SP_CalcPowerS(float* pCurrentMagBaseHarmonic, float* pVoltageMagBaseHarmonic, uint16_t numHarmonic, float* currentRms, float* voltageRms, float* powerS)
{
    // I = sqrt(I1^2 + I2^2, ... )  U = sqrt(U1^2 + U2^2, ... )
    *currentRms = SP_CalcSumSqrt(pCurrentMagBaseHarmonic, numHarmonic);
    *voltageRms = SP_CalcSumSqrt(pVoltageMagBaseHarmonic, numHarmonic);
    *powerS = (*currentRms) * (*voltageRms);
}

/**********************************************************************
 * 计算有功功率
 * 输入：pCurrentMagBaseHarmonic [电流基波, 谐波幅度], pVoltageMagBaseHarmonic [电压基波, 谐波幅度], pCurrentAngleBaseHarmonic [电流基波, 谐波幅度], pVoltageAngleBaseHarmonic [电压基波, 谐波幅度], numHarmonic [谐波次数]
 * 输出：有功功率
 */
float SP_CalcPowerP(float* pCurrentMagBaseHarmonic, float* pVoltageMagBaseHarmonic, float* pCurrentAngleBaseHarmonic, float* pVoltageAngleBaseHarmonic, uint16_t numHarmonic)
{
    float powerP = 0;
    float deltaAngle = 0;
    uint16_t i = 0;
    for(i = 0; i < numHarmonic; i++)
    {
        deltaAngle = fabs(pCurrentAngleBaseHarmonic[i] - pVoltageAngleBaseHarmonic[i]) * PI / 180.0f;
        powerP = powerP + pCurrentMagBaseHarmonic[i] * pVoltageMagBaseHarmonic[i] * cos(deltaAngle);
    }
    return powerP;
}
