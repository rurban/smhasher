# CMake generated Testfile for 
# Source directory: /home/cris7fe/smhasher
# Build directory: /home/cris7fe/smhasher/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(VerifyAll "SMHasher" "--test=VerifyAll")
add_test(Sanity "SMHasher" "--test=Sanity")
add_test(Speed "SMHasher" "--test=Speed")
add_test(Cyclic "SMHasher" "--test=Cyclic")
add_test(Zeroes "SMHasher" "--test=Zeroes")
add_test(Seed "SMHasher" "--test=Seed")
