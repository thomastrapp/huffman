#include "gtest/gtest.h"

#include "hm/encode-tree/main.h"
#include "hm/decode-tree/main.h"
#include "hm/common/main.h"
#include "hm/encode/main.h"
#include "hm/decode/main.h"
#include "hlp/main.h"

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
