#include <unity.h>
#include "kdu_protocol.h"
#include "freq_math.h"
#include "param_marshal.h"
#include <string.h>

CHAN_ARV chan_arv[ARV_MEM_COUNT]; // firmware defines this in main_fun.cpp; tests define their own

void setUp(void) {}
void tearDown(void) {}

// ---- kdu_protocol: the compile-time pins re-checked at runtime (belt and braces)
void test_frame_layout(void)
{
    TEST_ASSERT_EQUAL_INT(12, TX_RANK);
    TEST_ASSERT_EQUAL_INT(75, NOWSELCHAN_RANK);
    TEST_ASSERT_EQUAL_INT(93, BUF_SIZE);
}

void test_kdu_byte_encoding(void)
{
    TEST_ASSERT_EQUAL_INT('7', kdu_send_data(7));
    TEST_ASSERT_EQUAL_INT(7, kdu_recv_data('7'));
}

// ---- freq_math: characterization of checkFreqFloat behavior
void test_freq_on_5k_grid_passes_through(void)
{
    TEST_ASSERT_EQUAL_DOUBLE(435.55, checkFreqFloatStep(435.55, 0));
}

void test_freq_on_625_grid_passes_through(void)
{
    TEST_ASSERT_EQUAL_DOUBLE(430.10625, checkFreqFloatStep(430.10625, 1));
}

void test_freq_past_bug_430_13751_corrected(void)
{
    // The comment at src/main_fun.cpp:1632 records 430.13751 as a value that
    // once wrongly passed. Pin today's corrected result on the 5 kHz step.
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 430.14, checkFreqFloatStep(430.13751, 0));
}

void test_freq_corrects_to_625_grid(void)
{
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, 430.1375, checkFreqFloatStep(430.139, 1));
}

// ---- param_marshal: registry <-> CHAN_ARV round-trip
void test_chan_roundtrip(void)
{
    CHAN_ARV a, b;
    memset((void *)&a, 0, sizeof a);
    memset((void *)&b, 0, sizeof b);
    a.CHAN = 42; a.RX_FREQ = 435.55; a.TX_FREQ = 431.125;
    a.RS = 12; a.TS = 38; a.POWER = 1; a.GBW = 0;
    snprintf((char *)a.NN, sizeof a.NN, "%s", "TESTNN7");

    writeChanToArray(&a);
    readChanFromArray(&b);

    TEST_ASSERT_EQUAL_INT(42, b.CHAN);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 435.55, b.RX_FREQ);
    TEST_ASSERT_DOUBLE_WITHIN(1e-6, 431.125, b.TX_FREQ);
    TEST_ASSERT_EQUAL_INT(12, b.RS);
    TEST_ASSERT_EQUAL_INT(38, b.TS);
    TEST_ASSERT_EQUAL_INT(1, b.POWER);
    TEST_ASSERT_EQUAL_INT(0, b.GBW);
    TEST_ASSERT_EQUAL_STRING("TESTNN7", (const char *)b.NN);
}

void test_overlong_nickname_truncates_into_NN(void)
{
    // valStr can hold 15 chars; NN is 8 bytes (7 + '\0'). An unbounded copy
    // used to overflow into the neighboring CHAN_ARV memory.
    CHAN_ARV b;
    memset((void *)&b, 0, sizeof b);
    snprintf(parameterValue[Jnickname].valStr,
             sizeof(parameterValue[Jnickname].valStr), "%s", "ABCDEFGHIJKLMNO");
    snprintf(parameterValue[Jcurrent].valStr,
             sizeof(parameterValue[Jcurrent].valStr), "%s", "001");

    readChanFromArray(&b);

    TEST_ASSERT_EQUAL_STRING("ABCDEFG", (const char *)b.NN);
}

void test_unterminated_NN_reads_at_most_7(void)
{
    // If NN ever loses its terminator, the reverse copy must not read past
    // the 8-byte field.
    CHAN_ARV a;
    memset((void *)&a, 0, sizeof a);
    memset((void *)a.NN, 'X', sizeof a.NN); // no '\0' anywhere in NN

    writeChanToArray(&a);

    TEST_ASSERT_EQUAL_STRING("XXXXXXX", parameterValue[Jnickname].valStr);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_frame_layout);
    RUN_TEST(test_kdu_byte_encoding);
    RUN_TEST(test_freq_on_5k_grid_passes_through);
    RUN_TEST(test_freq_on_625_grid_passes_through);
    RUN_TEST(test_freq_past_bug_430_13751_corrected);
    RUN_TEST(test_freq_corrects_to_625_grid);
    RUN_TEST(test_chan_roundtrip);
    RUN_TEST(test_overlong_nickname_truncates_into_NN);
    RUN_TEST(test_unterminated_NN_reads_at_most_7);
    return UNITY_END();
}
