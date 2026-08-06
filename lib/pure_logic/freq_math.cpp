#include "freq_math.h"

/// @brief 频率校验
/// @param freq_buf 修改的频率
/// @return 校正的频率
double checkFreqFloatStep(double freq_buf, unsigned char step)
{
    int mul = 0;
    const int num = 2;
    int step_temp[num] = {500, 625};

    // Serial.printf("\nchan_arv[NOW].RX_FREQ:%lf\n", chan_arv[NOW].RX_FREQ);
    // Serial.printf("freq_buf:%lf\n", freq_buf);

    int freq_int = freq_buf * 10;
    // int freq_tail = (freq_buf * 10 - freq_int) * 10000;//错误通过:430.13751
    int freq_tail = freq_buf * 100000 - freq_int * 10000;

    // Serial.printf("freq_int:%d\n", freq_int);
    // Serial.printf("freq_tail:%d\n", freq_tail);

    for (int i = 0; i < num; i++)
    {
        if (freq_tail % step_temp[i] == 0)
            return freq_buf;
    }

    // 默认以设置的步进校正频率
    //  mul = (int)(freq_tail / step_temp[STEP] + 5) / 10;
    mul = (int)(freq_tail * 10 / step_temp[step] + 5) / 10;

    // Serial.printf("mul:%d, return: %lf\n", mul, mul * step_temp[STEP] / 100000.0 + freq_int / 10.0);

    return (mul * step_temp[step] / 100000.0 + freq_int / 10.0);
}
