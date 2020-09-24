#define DIR_NONE  0x00
#define DIR_CW    0x10
#define DIR_CCW   0x20  

#define R_START       0x0

// Use the half-step state table (emits a code at 00 and 11)
#define R_HALF_CCW_BEGIN   0x1
#define R_HALF_CW_BEGIN    0x2
#define R_HALF_START_M     0x3
#define R_HALF_CW_BEGIN_M  0x4
#define R_HALF_CCW_BEGIN_M 0x5

const unsigned char ttable_half[][4] = 
{
  // 00                  01              10            11
  {R_HALF_START_M,           R_HALF_CW_BEGIN,     R_HALF_CCW_BEGIN,  R_START},           // R_START (00)
  {R_HALF_START_M | DIR_CCW, R_START,             R_HALF_CCW_BEGIN,  R_START},           // R_CCW_BEGIN
  {R_HALF_START_M | DIR_CW,  R_HALF_CW_BEGIN,     R_START,           R_START},           // R_CW_BEGIN
  {R_HALF_START_M,           R_HALF_CCW_BEGIN_M,  R_HALF_CW_BEGIN_M, R_START},           // R_START_M (11)
  {R_HALF_START_M,           R_HALF_START_M,      R_HALF_CW_BEGIN_M, R_START | DIR_CW},  // R_CW_BEGIN_M 
  {R_HALF_START_M,           R_HALF_CCW_BEGIN_M,  R_HALF_START_M,    R_START | DIR_CCW}  // R_CCW_BEGIN_M
};

// Use the full-step state table (emits a code at 00 only)
#define R_CW_FINAL   0x1
#define R_CW_BEGIN   0x2
#define R_CW_NEXT    0x3
#define R_CCW_BEGIN  0x4
#define R_CCW_FINAL  0x5
#define R_CCW_NEXT   0x6

const unsigned char ttable[][4] = 
{
  // 00         01           10           11
  {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},           // R_START
  {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},  // R_CW_FINAL
  {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},           // R_CW_BEGIN
  {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},           // R_CW_NEXT
  {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},           // R_CCW_BEGIN
  {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW}, // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START}            // R_CCW_NEXT
};
