// RUN: %clang_cc1 -triple i386-pc-linux-gnu -emit-llvm -o - %s | FileCheck -check-prefix=CHECKBASIC %s
// RUN: %clang_cc1 -triple armv7a-eabi -mfloat-abi hard -emit-llvm -o - %s | FileCheck -check-prefix=CHECKCC %s

int g0;
// CHECKBASIC: @g0 = common global i32 0
static int bar1 = 42;
// CHECKBASIC: @bar1 = internal global i32 42

extern int g1;
extern int g1 __attribute((alias("g0")));
// CHECKBASIC-DAG: @g1 = alias i32* @g0

void f0(void) { }
extern void f1(void);
extern void f1(void) __attribute((alias("f0")));
// CHECKBASIC-DAG: @f1 = alias void ()* @f0
// CHECKBASIC-DAG: @test8_foo = weak alias bitcast (void ()* @test8_bar to void (...)*)
// CHECKBASIC-DAG: @test8_zed = alias bitcast (void ()* @test8_bar to void (...)*)
// CHECKBASIC-DAG: @test9_zed = alias void ()* @test9_bar
// CHECKBASIC: define void @f0() [[NUW:#[0-9]+]] {

// Make sure that aliases cause referenced values to be emitted.
// PR3200
static inline int foo1() { return 0; }
// CHECKBASIC-LABEL: define internal i32 @foo1()
int foo() __attribute__((alias("foo1")));
int bar() __attribute__((alias("bar1")));

extern int test6();
void test7() { test6(); }  // test6 is emitted as extern.

// test6 changes to alias.
int test6() __attribute__((alias("test7")));

static int inner(int a) { return 0; }
static int inner_weak(int a) { return 0; }
extern __typeof(inner) inner_a __attribute__((alias("inner")));
static __typeof(inner_weak) inner_weak_a __attribute__((weakref, alias("inner_weak")));
// CHECKCC: @inner_a = alias i32 (i32)* @inner
// CHECKCC: define internal arm_aapcs_vfpcc i32 @inner(i32 %a) [[NUW:#[0-9]+]] {

int outer(int a) { return inner(a); }
// CHECKCC: define arm_aapcs_vfpcc i32 @outer(i32 %a) [[NUW]] {
// CHECKCC: call arm_aapcs_vfpcc  i32 @inner(i32 %{{.*}})

int outer_weak(int a) { return inner_weak_a(a); }
// CHECKCC: define arm_aapcs_vfpcc i32 @outer_weak(i32 %a) [[NUW]] {
// CHECKCC: call arm_aapcs_vfpcc  i32 @inner_weak(i32 %{{.*}})
// CHECKCC: define internal arm_aapcs_vfpcc i32 @inner_weak(i32 %a) [[NUW]] {

// CHECKBASIC: attributes [[NUW]] = { nounwind{{.*}} }

// CHECKCC: attributes [[NUW]] = { nounwind{{.*}} }

void test8_bar() {}
void test8_foo() __attribute__((weak, alias("test8_bar")));
void test8_zed() __attribute__((alias("test8_foo")));

void test9_bar(void) { }
void test9_zed(void) __attribute__((section("test")));
void test9_zed(void) __attribute__((alias("test9_bar")));
