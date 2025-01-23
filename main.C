#include <iostream>
#include <fstream>
#include <climits>
#include <unistd.h>
#include "Tools.h"

static int doVerbose = 0;
static int doExit = 0;

//assert is a builtin macro.
//This is our own version of assert which provides more info.
#define myAssert(tst, res) \
   ({ \
      bool result = (tst == res); \
      if (!result) { \
         if (doVerbose || doExit) \
         { \
            std::cout << __func__ << ": failed on line " << std::dec << __LINE__ << " in " << __FILE__; \
            std::cout << ", Expected " << std::hex << res << ", Got " << std::hex << tst << "\n"; \
         } \
         if (doExit) \
         { \
            std::cout << "Aborting after first failed test\n"; \
            exit(1); \
         } \
      } \
      result; \
   })


/* static makes these local to this file */
/* prototypes for the testing functions */
static int buildLongTests();
static int getByteTests();
static int getBitsTests();
static int setBitsTests();
static int clearBitsTests();
static int signTests();
static int addOverflowTests();
static int subOverflowTests();
static int copyBitsTests();
static int setByteTests();

/* code for parsing the command line arguments */
static void parseArgs(int argc, char * argv[]);
static void usage(char * prog);

/* If you execute this program like this:
 * ./lab1 -v
 * then the program will output a message about each test
 * that fails and keep going.
 *
 * If you execute this program like this:
 * ./lab1 -e
 * The program will abort after the first failed test.
 *
 * If you execute this program like this:
 * ./lab1
 * then the program will continue execution even if a test fails,
 * but provide abbreviated output.
*/
int main(int argc, char * argv[])
{
   int numFuns = 10;
   int funsPassed = 0;
   std::string check[] = { "\x1B[31m fail \x1B[0m", "\x1B[32m pass \x1B[0m"};

   parseArgs(argc, argv);

   int pass; 
   pass = buildLongTests();
   std::cout << "buildLong tests:" << check[pass] << "\n";
   funsPassed += pass;
   
   pass = getByteTests();
   std::cout << "getByte tests:" << check[pass] << "\n";
   funsPassed += pass;

   pass = getBitsTests();
   std::cout << "getBits tests:" << check[pass] << "\n";
   funsPassed += pass;

   pass = setBitsTests();
   std::cout << "setBits tests:" << check[pass] << "\n";
   funsPassed += pass;

   pass = clearBitsTests();
   std::cout << "clearBits tests:" << check[pass] << "\n";
   funsPassed += pass;

   pass = signTests();
   std::cout << "sign tests:" << check[pass] << "\n";
   funsPassed += pass;

   pass = addOverflowTests();
   std::cout << "addOverflow tests:" << check[pass] << "\n";
   funsPassed += pass;

   pass = subOverflowTests();
   std::cout << "subOverflow tests:" << check[pass] << "\n";
   funsPassed += pass;

   pass = setByteTests();
   std::cout << "setByte tests:" << check[pass] << "\n";
   funsPassed += pass;

   pass = copyBitsTests();
   std::cout << "copyBits tests:" << check[pass] << "\n";
   funsPassed += pass;

   std::cout << "\n" << std::dec << funsPassed << " functions out of a total of " << numFuns
             << " passed their tests\n";
}

/**
 * parse the command line arguments to set the doVerbose and doExit flags
 * or usage information
 */
void parseArgs(int argc, char * argv[])
{
   char opt;
   int count = 0;
   if (argc == 1) return; //no arguments
   while((opt = getopt(argc, argv, "hve")) != -1)
   {
      count++;
      switch (opt)
      {
         case 'v':
            doVerbose = 1;
            break;
         case 'e':
            doExit = 1;
            break;
         case '?':
         case 'h':
            usage(argv[0]);
      }
   }
   if (count < argc - 1) 
   {
      std::cout << "bad arguments\n";
      usage(argv[0]);
   }
}

/**
 * print usage info and exit
 */
void usage(char * prog)
{
   std::cout << "usage: " << prog << " [-h] [-v] [-e]\n";
   std::cout << "       -h print help info and exit\n";
   std::cout << "       -v print info about each failing test\n";
   std::cout << "       -e exit after failed test\n";
   exit(1);
}

/**
 * tests the addOverflow method in the Tools class
 *  
 * addOverflow assumes that op1 and op2 contain 64 bit two's 
 * complement values and returns true if an overflow would 
 * occur if they are summed and false otherwise
 * bool Tools::addOverflow(uint64_t op1, uint64_t op2)
 */
int addOverflowTests()
{
   int pass;
   pass = myAssert(Tools::addOverflow(0xffffffffffffffff, 0xffffffffffffffff), false);
   pass &= myAssert(Tools::addOverflow(0x8000000000000000, 0x8000000000000000), true);

   pass &= myAssert(Tools::addOverflow(31, 0x7fffffffffffffe1ul), true);
   pass &= myAssert(Tools::addOverflow(31, 0x7fffffffffffffe0ul), false);
   pass &= myAssert(Tools::addOverflow(-15, 0x800000000000000e), true);
   pass &= myAssert(Tools::addOverflow(-15, 0x800000000000000f), false);
   
   return pass;
}

/**
 * tests the subOverflow method in the Tools class
 *
 * subOverflow assumes that op1 and op2 contain 64 bit two's 
 * complement values and returns true if an overflow would occur 
 * from op2 - op1 and false otherwise
 * bool Tools::subOverflow(uint64_t op1, uint64_t op2)
*/
int subOverflowTests()
{
   int pass;
   pass = myAssert(Tools::subOverflow(0xffffffffffffffff, 0xffffffffffffffff), false);
   pass &= myAssert(Tools::subOverflow(0x0000000000000004, 0x8000000000000003), true);
   pass &= myAssert(Tools::subOverflow(0x8000000000000008, 8), true);
   pass &= myAssert(Tools::subOverflow(0x8000000000000009, 8), false);
   pass &= myAssert(Tools::subOverflow(0x8000000000000000, 0x8000000000000001), false);
   pass &= myAssert(Tools::subOverflow(0x7ffffffffffffff2ul, -15), true);
   pass &= myAssert(Tools::subOverflow(0x7ffffffffffffff1ul, -15), false);

   return pass;
}


/** 
 * tests the buildLong method in the Tools class.
 *
 * buildLong takes an array of unsigned chars (uint8_t)
 * and returns an unsigned long (uint64_t) built from
 * the bytes in the array ordering such that the low
 * order byte is the 0th element of the array 
 * uint64_t Tools::buildLong(uint8_t bytes[LONGSIZE]);
*/
int buildLongTests()
{
   int pass;
   uint8_t bytes1[LONGSIZE] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
   pass = myAssert(Tools::buildLong(bytes1), 0x8877665544332211);

   uint8_t bytes2[LONGSIZE] = {0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
   pass &= myAssert(Tools::buildLong(bytes2), 0x1122334455667788);

   uint8_t bytes3[LONGSIZE] = {0x78, 0x67, 0x56, 0x45, 0x34, 0x23, 0x12, 0x01};
   pass &= myAssert(Tools::buildLong(bytes3), 0x0112233445566778);

   uint8_t bytes4[LONGSIZE] = {0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};
   pass &= myAssert(Tools::buildLong(bytes4), 0x0102030405060708);

   uint8_t bytes5[LONGSIZE] = {0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20, 0x10};
   pass &= myAssert(Tools::buildLong(bytes5), 0x1020304050607080);

   uint8_t bytes6[LONGSIZE] = {0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff};
   pass &= myAssert(Tools::buildLong(bytes6), 0xffffffff00000000);

   uint8_t bytes7[LONGSIZE] = {0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00};
   pass &= myAssert(Tools::buildLong(bytes7), 0x00000000ffffffff); 
   
   return pass;
}

/**
 * tests the getByte method in the Tools class
 *
 * getByte takes an uint64_t as input that is the source 
 * and an int32_t containing a byte number in the range 0 to 7.
 * getByte returns the numbered byte from the source. If the byte number
 * is out of range, getByte returns 0.
 * uint64_t Tools::getByte(uint64_t ul, int32_t byteNum);
*/
int getByteTests()
{
   /* getByte tests */
   int pass;
   pass = myAssert(Tools::getByte(0x8877665544332211, 0), 0x11);
   pass &= myAssert(Tools::getByte(0x8877665544332211, 1), 0x22);
   pass &= myAssert(Tools::getByte(0x8877665544332211, 2), 0x33);
   pass &= myAssert(Tools::getByte(0x8877665544332211, 3), 0x44);
   pass &= myAssert(Tools::getByte(0x8877665544332211, 4), 0x55);
   pass &= myAssert(Tools::getByte(0x8877665544332211, 5), 0x66);
   pass &= myAssert(Tools::getByte(0x8877665544332211, 6), 0x77);
   pass &= myAssert(Tools::getByte(0x8877665544332211, 7), 0x88);
   pass &= myAssert(Tools::getByte(0x1234567821436587, 0), 0x87);
   pass &= myAssert(Tools::getByte(0x1234567821436587, 1), 0x65);
   pass &= myAssert(Tools::getByte(0x1234567821436587, 2), 0x43);
   pass &= myAssert(Tools::getByte(0x1234567821436587, 3), 0x21);
   pass &= myAssert(Tools::getByte(0x1234567821436587, 4), 0x78);
   pass &= myAssert(Tools::getByte(0x1234567821436587, 5), 0x56);
   pass &= myAssert(Tools::getByte(0x1234567821436587, 6), 0x34);
   pass &= myAssert(Tools::getByte(0x1234567821436587, 7), 0x12);
   pass &= myAssert(Tools::getByte(0x1234567821436587, 8), 0x00);
   pass &= myAssert(Tools::getByte(0x1234567821436587, -1), 0x00);
   pass &= myAssert(Tools::getByte(0x1234567821436587, 0x7fffffff), 0x00);
   pass &= myAssert(Tools::getByte(0x1234567821436587, 9), 0x00);
   pass &= myAssert(Tools::getByte(0x1234567821436587, 10), 0x00);
   return pass;
}

/**
 * tests the getBits method in the Tools class
 *
 * getBits accepts as input an uint64_t and two ints that
 * indicate a range of bits to grab and returns the bits
 * low through high of the uint64_t.  bit 0 is the low order bit
 * and bit 63 is the high order bit. returns 0 if the low or 
 * high bit numbers are out of range
 * uint64_t Tools::getBits(uint64_t source, int32_t low, int32_t high);
*/
int getBitsTests()
{
   int pass;
   pass = myAssert(Tools::getBits(0x1122334455667788, 0, 7), 0x88);
   pass &= myAssert(Tools::getBits(0x1122334455667788, 8, 0xf), 0x77);
   pass &= myAssert(Tools::getBits(0x1122334455667788, 0x10, 0x17), 0x66);
   pass &= myAssert(Tools::getBits(0x1122334455667788, 0x18, 0x1f), 0x55);
   pass &= myAssert(Tools::getBits(0x1122334455667788, 0x20, 0x27), 0x44);
   pass &= myAssert(Tools::getBits(0x1122334455667788, 0x28, 0x2f), 0x33);
   pass &=  myAssert(Tools::getBits(0x1122334455667788, 0x30, 0x37), 0x22);
   pass &= myAssert(Tools::getBits(0x1122334455667788, 0x38, 0x3f), 0x11);
   pass &= myAssert(Tools::getBits(0x1122334455667788, 0, 0x3f), 0x1122334455667788);
   pass &= myAssert(Tools::getBits(0x1122334455667788, 0x3f, 0), 0);
   pass &= myAssert(Tools::getBits(0xffffffffffffffff, 0x3f, 0x3f), 1);
   pass &= myAssert(Tools::getBits(0x8000000000000001, 0, 0), 1);
   pass &= myAssert(Tools::getBits(0x700000000000000e, 0, 0), 0);
   pass &= myAssert(Tools::getBits(0x700000000000000e, 0x3f, 0x3f), 0);
   return pass;
}

/**
 * tests the setBits method in the Tools class
 *
 * setBits sets the bits of source in the range specified by the low and high
 * parameters to 1 and returns that value. returns source if the low or high
 * bit numbers are out of range
 * uint64_t Tools::setBits(uint64_t source, int32_t low, int32_t high)
*/
int setBitsTests()
{
   int pass;
   pass = myAssert(Tools::setBits(0x1122334455667788, 0, 7), 0x11223344556677ff);
   pass &= myAssert(Tools::setBits(0x1122334455667788, 8, 0xf), 0x112233445566ff88);
   pass &= myAssert(Tools::setBits(0x1122334455667788, 0x10, 0x17), 0x1122334455ff7788);
   pass &= myAssert(Tools::setBits(0x1122334455667788, 0x18, 0x1f), 0x11223344ff667788);
   pass &= myAssert(Tools::setBits(0x1122334455667788, 0x20, 0x27), 0x112233ff55667788);
   pass &= myAssert(Tools::setBits(0x1122334455667788, 0x28, 0x2f), 0x1122ff4455667788);
   pass &= myAssert(Tools::setBits(0x1122334455667788, 0x30, 0x37), 0x11ff334455667788);
   pass &= myAssert(Tools::setBits(0x1122334455667788, 0x38, 0x3f), 0xff22334455667788);
   pass &= myAssert(Tools::setBits(0x1122334455667788, 0, 0x3f), 0xffffffffffffffff);
   pass &= myAssert(Tools::setBits(0x1122334455667788, 0x3f, 0), 0x1122334455667788);
   pass &= myAssert(Tools::setBits(0x7fffffffffffffff, 0x3f, 0x3f),  0xffffffffffffffff);
   pass &= myAssert(Tools::setBits(0xfffffffffffffffe, 0, 0), 0xffffffffffffffff);
   pass &= myAssert(Tools::setBits(0x0000000000000001, 0x3f, 0x3f), 0x8000000000000001);
   pass &= myAssert(Tools::setBits(0x8000000000000001, 0x3f, 0x3f), 0x8000000000000001);
   pass &= myAssert(Tools::setBits(0x8000000000000000, 0, 0), 0x8000000000000001);
   pass &= myAssert(Tools::setBits(0x8000000000000001, 0, 0), 0x8000000000000001);
   pass &= myAssert(Tools::setBits(0x8000000000000001, 2, 3), 0x800000000000000d);
   return pass;
}

/**
 * tests the clearBits method in the Tools class
 *
 * clearBits sets the bits of source in the range low
 * to high to 0 (clears them) and returns that value. 
 * returns source if the low or high
 * bit numbers are out of range
 * uint64_t Tools::clearBits(uint64_t source, int32_t low, int32_t high)
*/
int clearBitsTests()
{
   int pass;
   pass = myAssert(Tools::clearBits(0x1122334455667788, 0, 7), 0x1122334455667700);
   pass &= myAssert(Tools::clearBits(0x1122334455667788, 4, 7), 0x1122334455667708);
   pass &= myAssert(Tools::clearBits(0x1122334455667788, 8, 0xf), 0x1122334455660088);
   pass &= myAssert(Tools::clearBits(0x1122334455667788, 0x10, 0x13), 0x1122334455607788);
   pass &= myAssert(Tools::clearBits(0x1122334455667789, 0, 0), 0x1122334455667788);
   pass &= myAssert(Tools::clearBits(0x9122334455667788, 0x3f, 0x3f), 0x1122334455667788);
   pass &= myAssert(Tools::clearBits(0x1122334455667788, 0x30, 0x3f), 0x0000334455667788);
   pass &= myAssert(Tools::clearBits(0x1122334455667788, 0x40, 0x3f), 0x1122334455667788);
   pass &= myAssert(Tools::clearBits(0x1122334455667788, 0x30, 0x40), 0x1122334455667788);
   return pass;
}

/**
 * tests the sign method in the Tools class
 *
 * sign assumes source contains a 64 bit two's complement value and
 * returns the sign (1 or 0)
 * uint64_t Tools::sign(uint64_t source)
*/
int signTests()
{
   int pass;
   pass = myAssert(Tools::sign(0x1122334455667788), 0);
   pass &= myAssert(Tools::sign(0x8877665544332211), 1);
   pass &= myAssert(Tools::sign(0x0000000000000000), 0);
   pass &= myAssert(Tools::sign(0x1111111111111111), 0);
   pass &= myAssert(Tools::sign(0xffffffffffffffff), 1);
   return pass;
}


/**
 * tests the copyBits method in the Tools class
 *
 * copyBits copies length bits from the source to a destination and returns the
 * modified destination. If low bit number of the source or
 * dest is out of range or the calculated source or dest high bit
 * number is out of range, then the unmodified destination is returned.
 * uint64_t Tools::copyBits(uint64_t source, uint64_t dest,
 *                          int32_t srclow, int32_t dstlow, int32_t length)
*/
int copyBitsTests()
{
   int pass;

   pass = myAssert(Tools::sign(0x1122334455667788), 0);

   pass &= myAssert(Tools::copyBits(0x1122334455667788, 0x8877665544332211, 0, 0, 8),
         0x8877665544332288);
   pass &= myAssert(Tools::copyBits(0x1122334455667788, 0x8877665544332211, 0, 8, 8),
         0x8877665544338811);
   pass &= myAssert(Tools::copyBits(0x1122334455667788, 0x8877665544332211, 8, 4, 4),
         0x8877665544332271);
   pass &= myAssert(Tools::copyBits(0x1122334455667788, 0x1877665544332211, 3, 0x3f, 1),
         0x9877665544332211);
   pass &= myAssert(Tools::copyBits(0x1122334455667788, 0x1877665544332211, 3, 0x40, 1),
         0x1877665544332211);
   pass &= myAssert(Tools::copyBits(0x1122334455667788, 0x1877665544332211, 0xfffffffe, 
          0x3f, 1), 0x1877665544332211);
   pass &= myAssert(Tools::copyBits(0x1122334455667788, 0x1877665544332211, 3, 0x3f, 2),
         0x1877665544332211);
   pass &= myAssert(Tools::copyBits(0x1122334455667788, 0x8877665544332211, 8, 8, 0x10),
         0x8877665544667711);


   return pass;
}

/**
 * tests the setByte method in the Tools class
 *
 * setByte sets the bits of source identfied by the byte number to 1 and
 * returns that value. if the byte number is out of range then source
 * is returned unchanged.
 * uint64_t Tools::setByte(uint64_t source, int32_t byteNum)
*/
int setByteTests()
{
   int pass;
   pass = myAssert(Tools::setByte(0x1122334455667788, 0), 0x11223344556677ff);
   pass &= myAssert(Tools::setByte(0x1122334455667788, 1), 0x112233445566ff88);
   pass &= myAssert(Tools::setByte(0x1122334455667788, 2), 0x1122334455ff7788);
   pass &= myAssert(Tools::setByte(0x1122334455667788, 3), 0x11223344ff667788);
   pass &= myAssert(Tools::setByte(0x1122334455667788, 4), 0x112233ff55667788);
   pass &= myAssert(Tools::setByte(0x1122334455667788, 5), 0x1122ff4455667788);
   pass &= myAssert(Tools::setByte(0x1122334455667788, 6), 0x11ff334455667788);
   pass &= myAssert(Tools::setByte(0x1122334455667788, 7), 0xff22334455667788);
   pass &= myAssert(Tools::setByte(0x1122334455667788, 8), 0x1122334455667788);
   pass &= myAssert(Tools::setByte(0x1122334455667788, -1), 0x1122334455667788);
   pass &= myAssert(Tools::setByte(0x1122334455667788, -2), 0x1122334455667788);
   pass &= myAssert(Tools::setByte(0x0000000000000000, 7), 0xff00000000000000);
   pass &= myAssert(Tools::setByte(0x0000000000000000, 6), 0x00ff000000000000);
   pass &= myAssert(Tools::setByte(0x0000000000000000, 5), 0x0000ff0000000000);
   pass &= myAssert(Tools::setByte(0x0000000000000000, 4), 0x000000ff00000000);
   pass &= myAssert(Tools::setByte(0x0000000000000000, 3), 0x00000000ff000000);
   pass &= myAssert(Tools::setByte(0x0000000000000000, 2), 0x0000000000ff0000);
   pass &= myAssert(Tools::setByte(0x0000000000000000, 1), 0x000000000000ff00);
   pass &= myAssert(Tools::setByte(0x0000000000000000, 0), 0x00000000000000ff);
   pass &= myAssert(Tools::setByte(0x0000000000000000, 8), 0x0000000000000000);
   pass &= myAssert(Tools::setByte(0x0000000000000000, -1), 0x0000000000000000);
   pass &= myAssert(Tools::setByte(0x0023000000000000, -2), 0x0023000000000000);
   pass &= myAssert(Tools::setByte(0x0023000000000000, 0x7fffffff), 0x0023000000000000);

   return pass;
}
