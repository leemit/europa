//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software.

/**
   @file module-tests.cc
   @author Will Edgington

   @brief A small test of classes Error and TestData and the related
   macros.

   CMG: Added test for id's and entities
*/

#include "util-test-module.hh"
#include "Error.hh"
//#include "Debug.hh"
//#include "LoggerTest.hh"
#include "LabelStr.hh"
#include "TestData.hh"
#include "Id.hh"
#include "Entity.hh"
#include "XMLUtils.hh"
#include "Number.hh"
#include "Engine.hh"
#include "tinyxml.h"
#include "CommonDefs.hh"

#include <list>
#include <sstream>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <typeinfo>

// using EUROPA::Utils::test::LoggerTest;
// using EUROPA::Utils::Logger;

#ifndef EUROPA_FAST
#define non_fast_only_assert(T) CPPUNIT_ASSERT(T)
#else
#define non_fast_only_assert(T) //NO-OP
#endif

#undef EUROPA_runTest
#define EUROPA_runTest(test) { \
  try { \
    std::cout << "      " << #test; \
    bool result = test(); \
    if (result) \
      std::cout << " PASSED." << std::endl; \
    else { \
      std::cout << " FAILED TO PASS UNIT TEST." << std::endl; \
      throw Error::GeneralUnknownError(); \
    } \
  } \
  catch (Error err) { \
    err.print(std::cout); \
  } \
}

using namespace EUROPA;

class TestError {
public:
  static const char* TEST_CONST() {
    static const char* sl_value = "TestData";
    return sl_value;
  }
  DECLARE_ERROR(BadThing);
};

class ErrorTest {
public:
  static bool test() {
    EUROPA_runTest(testExceptions);
    return true;
  }
private:
  static bool testExceptions() {
    CPPUNIT_ASSERT(strcmp(TestError::TEST_CONST(), "TestData") == 0);
    bool success = true;
    Error::doThrowExceptions();
    int var = 1;
    CPPUNIT_ASSERT(var == 1);
    CPPUNIT_ASSERT(Error::printingErrors());
    CPPUNIT_ASSERT(Error::displayWarnings());
    CPPUNIT_ASSERT(Error::throwEnabled());
    try {
      // These are tests of check_error() and should therefore not be changed
      //   to CPPUNIT_ASSERT() despite the usual rule for test programs.
      // --wedgingt@email.arc.nasa.gov 2005 Feb 9
      check_error(Error::printingErrors(), "not printing errors by default!");
      check_error(Error::displayWarnings(), "display warnings off by default!");
      check_error(var == 1);
      check_error(var == 1, "check_error(var == 1)");
      check_error(var == 1, Error("check_error(var == 1)"));
      checkError(var ==1, "Can add " << 1.09 << " and " << 2.81 << " to get " << 1.09 +2.81);
      condWarning(var == 1, "var is not 1");
      std::cout << std::endl;
      Error::setStream(std::cout);
      europaWarn("Warning messages working");
      Error::setStream(std::cerr);
    }
    catch (Error e) {
      __x__(e);
      success = false;
    }
    // check_error will not throw the errors for EUROPA_FAST
#if !defined(EUROPA_FAST) && !defined(__CYGWIN__)
    CPPUNIT_ASSERT(Error::throwEnabled());
    /* Do not print errors that we are provoking on purpose to ensure they are noticed. */
    try {
      Error::doNotDisplayErrors();
      check_error(var == 2);
      __y__("check_error(var == 2) did not throw an exception");
      success = false;
    }
    catch (Error e) {
      Error::doDisplayErrors();
      __z__(e, Error("var == 2", __FILE__, __LINE__ - 5), success);
    }
    try {
      Error::doNotDisplayErrors();
      check_error(var == 2, "check_error(var == 2)");
      __y__("check_error(var == 2, blah) did not throw an exception");
      success = false;
    }
    catch (Error e) {
      Error::doDisplayErrors();
      __z__(e, Error("var == 2", "check_error(var == 2)", __FILE__, __LINE__ - 5), success);
    }
    try {
      Error::doNotDisplayErrors();
      check_error(var == 2, Error("check_error(var == 2)"));
      __y__("check_error(var == 2, Error(blah)) did not throw an exception");
      success = false;
    }
    catch (Error e) {
      Error::doDisplayErrors();
      __z__(e, Error("var == 2", "check_error(var == 2)", __FILE__, __LINE__ - 5), success);
    }
    try {
      Error::doNotDisplayErrors();
      check_error(var == 2, "check_error(var == 2)", TestError::BadThing());
      __y__("check_error(var == 2, TestError::BadThing()) did not throw an exception");
      success = false;
    }
    catch (Error e) {
      Error::doDisplayErrors();
      //!!Should, perhaps, be:
      //__z__(e, Error(TestError::BadThing(), __FILE__, __LINE__ - 7), success);
      // ... but is actually:
      __z__(e, Error("var == 2", "check_error(var == 2)", __FILE__, __LINE__ - 9), success);
    }
#endif
    return(success);
  }
};

class DebugTest {
public:
  static bool test() {
    EUROPA_runTest(testDebugError);
    EUROPA_runTest(testDebugFiles);
//     EUROPA_runTest(testLog4cpp);
//     EUROPA_runTest(testLogger);
    return true;
  }
private:

  static bool testDebugError() {
    bool success = true;
    // check_error will not throw the errors for EUROPA_FAST
#if !defined(EUROPA_FAST) && defined(DEBUG_MESSAGE_SUPPORT)
    Error::doThrowExceptions();
    CPPUNIT_ASSERT(Error::throwEnabled());
    //!!Add a test of DebugMessage that should throw an error here.
    //!!  Skipped for lack of time presently. --wedgingt@email.arc.nasa.gov
    Error::doNotThrowExceptions();
    CPPUNIT_ASSERT(!Error::throwEnabled());
#endif
    return(success);
  }

  static bool testDebugFiles() {
    for (int i = 1; i < 7; i++)
      runDebugTest(i);
    return(true);
  }
//   /** Tests that log4cpp functionality is installed and working */
//   static bool testLog4cpp() {
//     bool success = true;
// #if !defined(EUROPA_FAST) && defined(DEBUG_MESSAGE_SUPPORT)

// #endif
//     return(success);
//   }

  /** Tests that the Europa logger functionality is installed and working */
//   static bool testLogger() {
//     bool success = true;
// #if !defined(EUROPA_FAST) && defined(DEBUG_MESSAGE_SUPPORT)
//     LoggerTest *tester = new LoggerTest();
//     tester->testLogger();
// #endif
//     return(success);
//   }


  static void runDebugTest(int cfgNum) {
    std::cout << "Running runDebugTest(" << cfgNum << ")" << std::endl;

#if !defined(EUROPA_FAST) && defined(DEBUG_MESSAGE_SUPPORT)
    std::stringstream cfgName;
    cfgName << "../../Utils/test/debug" << cfgNum << ".cfg";
    std::string cfgFile(cfgName.str());
    cfgName << ".output";
    std::string cfgOut(cfgName.str());

    Error::doNotThrowExceptions();
//     Error::doNotDisplayErrors();
    std::ofstream debugOutput(cfgOut.c_str());
    CPPUNIT_ASSERT_MESSAGE("could not open debug output file", debugOutput.good());
    DebugMessage::setStream(debugOutput);
    std::ifstream debugStream(cfgFile.c_str());
    CPPUNIT_ASSERT_MESSAGE("could not open debug config file", debugStream.good()); //, DebugErr::DebugConfigError());
    if (!DebugMessage::readConfigFile(debugStream))
      handle_error(!DebugMessage::readConfigFile(debugStream),
                   "problems reading debug config file",
                   DebugErr::DebugConfigError());

    debugMsg("main1", "done opening files");
    condDebugMsg(std::cout.good(), "main1a", "std::cout is good");
    debugStmt("main2a", int s = 0; for (int i = 0; i < 5; i++) { s += i; } debugOutput << "Sum is " << s << '\n'; );
    debugMsg("main2", "primary testing done");
    Error::doThrowExceptions();
    Error::doDisplayErrors();
    DebugMessage::setStream(std::cerr);
#endif
  }


};

/**
 * Support classes to enable testing
 * Foo: Basic allocation and deallocation.
 * Bar: Casting behaviour
 * EmbeddedClass: Error handling and checking for release and alloction patterns with embedded id's.
 */
class Root {
public:
  Root() {
  }

  virtual ~Root() {
  }
};

class Foo: public Root {
public:
  Foo() {
    counter++;
  }

  virtual ~Foo() {
    counter--;
  }

  void increment() {
    counter++;
  }

  void decrement() {
    counter--;
  }

  bool doConstFunc() const {
    return(true);
  }

  static int getCount() {
    return(counter);
  }

private:
  static int counter;
};

int Foo::counter(0);

class Bar: public Foo {
public:
  Bar() {
  }
};

class Baz: public Foo
{
public:
  Baz(){
  }
};

class Bing: public Root
{
public:
  Bing(){
  }
};

namespace {
// void overloadFunc(const Id<Bing>&) {
//   CPPUNIT_ASSERT(true);
// }

// void overloadFunc(const Id<Foo>&) {
//   CPPUNIT_ASSERT(true);
// }
}

class IdTests {
public:
  static bool test();

private:
  static bool testBasicAllocation();
  static bool testTypicalConversionsAndComparisons();
  static bool testCollectionSupport();
  static bool testDoubleConversion();
  static bool testCastingSupport();
  static bool testMultipleInheritanceCasting();
  static bool testBadAllocationErrorHandling();
  static bool testBadIdUsage();
  static bool testIdConversion();
  static bool testConstId();
};

bool IdTests::test() {
  EUROPA_runTest(testBasicAllocation);
  EUROPA_runTest(testCollectionSupport);
  EUROPA_runTest(testDoubleConversion);
  EUROPA_runTest(testCastingSupport);
  // NOT APPLICABLE: EUROPA_runTest(testMultipleInheritanceCasting);
  EUROPA_runTest(testTypicalConversionsAndComparisons);
  EUROPA_runTest(testBadAllocationErrorHandling);
  EUROPA_runTest(testBadIdUsage);
  EUROPA_runTest(testIdConversion);
  EUROPA_runTest(testConstId);
  return(true);
}

bool IdTests::testBasicAllocation() {
#ifndef EUROPA_FAST
  unsigned long initialSize = IdTable::size();
#endif
  Foo *fooPtr = new Foo();
  Id<Foo> fId1(fooPtr);
  assert(fId1.isId());
  CPPUNIT_ASSERT(Foo::getCount() == 1);
  non_fast_only_assert(IdTable::size() == initialSize + 1);

  fId1->increment();
  CPPUNIT_ASSERT(Foo::getCount() == 2);
  fId1->decrement();
  CPPUNIT_ASSERT(Foo::getCount() == 1);

  Id<Foo> fId2 = fId1;
  CPPUNIT_ASSERT(Foo::getCount() == 1);

  CPPUNIT_ASSERT(fId1.isValid() && fId2.isValid());
  CPPUNIT_ASSERT(!fId1.isInvalid() && !fId2.isInvalid());

  fId2.release();
  CPPUNIT_ASSERT(Foo::getCount() == 0);
  non_fast_only_assert( fId1.isInvalid() &&  fId2.isInvalid());
  return true;
}


bool IdTests::testTypicalConversionsAndComparisons()
{
  Foo* foo1 = new Foo();
  Id<Foo> fId1(foo1);
  Id<Foo> fId2(fId1);
  CPPUNIT_ASSERT(fId1 == fId2); // Equality operator
  CPPUNIT_ASSERT(&*fId1 == &*fId2); // Dereferencing operator
  CPPUNIT_ASSERT(foo1 == &*fId2); // Dereferencing operator
  CPPUNIT_ASSERT(foo1 == static_cast<Foo*>(fId2)); // Dereferencing operator
  CPPUNIT_ASSERT(foo1 == fId2.operator->());
  CPPUNIT_ASSERT( ! (fId1 > fId2));
  CPPUNIT_ASSERT( ! (fId1 < fId2));

  Foo* foo2 = new Foo();
  Id<Foo> fId3(foo2);
  CPPUNIT_ASSERT(fId1 != fId3);

  fId1.release();
  fId3.release();
  return true;
}

bool IdTests::testCollectionSupport()
{
  // Test inclusion in a collection class - forces compilation test
  std::list< Id<Foo> > fooList;
  CPPUNIT_ASSERT(fooList.size() == 0);
  return true;
}

bool IdTests::testDoubleConversion()
{
//   Id<Foo> fId(new Foo());
//   double fooAsDouble = (double) fId;
//   Id<Foo> idFromDbl(fooAsDouble);
//   CPPUNIT_ASSERT(idFromDbl == fId);
//   fId.release();
  return true;
}

bool IdTests::testCastingSupport()
{
  Foo* foo = new Foo();
  Id<Foo> fId(foo);
  Foo* fooByCast = static_cast<Foo*>(fId);
  CPPUNIT_ASSERT(foo == fooByCast);

  CPPUNIT_ASSERT(Id<Bar>::convertable(fId) == false);
  fId.release();

  Foo* bar = new Bar();
  Id<Bar> bId(dynamic_cast<Bar*>(bar));
  fId = bId;
  CPPUNIT_ASSERT(Id<Bar>::convertable(fId) == true);
  bId.release();

  // bId = Id<Bar>(new Bar());
//   double ptrAsDouble = bId; // Cast to double

//   const Id<Bar>& cbId(ptrAsDouble);
//   CPPUNIT_ASSERT(cbId.isValid());
//   CPPUNIT_ASSERT(cbId == bId);
//   bId.release();
//   non_fast_only_assert(cbId.isInvalid());

  Id<Baz> fId1(new Baz());
  // DOES NOT COMPILE: overloadFunc(fId1);
  fId1.release();
  return true;
}

class X {
public:
  virtual ~X(){}
};

class Y {
public:
  virtual ~Y(){}
};

class Z: public X, public Y {};

bool IdTests::testMultipleInheritanceCasting(){
  Id<Z> z_id(new Z());
  CPPUNIT_ASSERT(Id<X>::convertable(z_id));
  CPPUNIT_ASSERT(Id<Y>::convertable(z_id));

  Id<X> x_id(z_id);
  Id<Y> y_id(z_id);

  CPPUNIT_ASSERT (x_id == y_id);
  z_id.release();

  return true;
}

bool IdTests::testBadAllocationErrorHandling()
{
  std::cout << std::endl;
  bool success = true;
  // check_error (inside class Id) will not throw the errors when compiled with EUROPA_FAST.
#ifndef EUROPA_FAST
  // Ensure allocation of a null pointer triggers error
  //LabelStr expectedError = IdErr::IdMgrInvalidItemPtrError();
  Error::doThrowExceptions();
#if !defined(__CYGWIN__)
  // This exception simply isn't being caught on Cygwin for some reason.
  try {
    Error::doNotDisplayErrors();
    Id<Foo> fId0(static_cast<Foo*>(NULL));
    CPPUNIT_ASSERT_MESSAGE("Id<Foo> fId0((Foo*) 0); failed to error out.", false);
    success = false;
  }
  catch (Error e) {
    Error::doDisplayErrors();
    // Path of Id.hh may vary depending on where test is run from.
    // Match only the filename and not the full path
    std::string pathMsg = e.getFile();
    std::string::size_type end = pathMsg.length();
    std::string name = "Id.hh";
    std::string::size_type start = pathMsg.find(name);
    if (start != std::string::npos) {
      std::string fileMsg = pathMsg.substr(start, end);
      e.setFile(fileMsg);
    }
    __z__(e, Error("ptr != 0", "Cannot generate an Id<3Foo> for 0 pointer.", "Id.hh", 0), success);
  }
#endif
  Error::doNotThrowExceptions();

  Foo* foo = new Foo();
  Id<Foo> fId1(foo);
  fId1.remove();
  Id<Foo> fId3(foo);
  fId3.release();
#endif

  return(success);
}

bool IdTests::testBadIdUsage() {
  bool success = true;
  Id<Root> barId(new Bar());
  Error::doThrowExceptions();
#if !defined(__CYGWIN__)
  // This exception isn't being caught on Cygwin.
  try {
    Error::doNotDisplayErrors();
    Id<Bing> bingId = barId;
#ifdef EUROPA_FAST
    CPPUNIT_ASSERT(bingId.isNoId());
#else
    CPPUNIT_ASSERT_MESSAGE("Id<Bing> bingId = barId; failed to error out.", false);
    success = false;
#endif
  }
  catch (Error e) {
    Error::doDisplayErrors();
    CPPUNIT_ASSERT_EQUAL(std::string("Error"), e.getType());
    CPPUNIT_ASSERT_EQUAL(std::string("Invalid cast from Id<4Root> to Id<4Bing>."),
                         e.getMsg());
  }
  catch (IdErr) {
    Error::doDisplayErrors();
    std::cerr << "Caught expected IdErr::IdMgrInvalidItemPtrError" << std::endl;
    // No operator==() implemented ...
    // __z__(idErr, IdErr::IdMgrInvalidItemPtrError(), success);
  }
#endif
  Error::doNotThrowExceptions();
  barId.release();
  return(success);
}

bool IdTests::testIdConversion()
{
  int count = Foo::getCount();
  Id<Foo> fooId(new Bar());
  Id<Bar> barId(fooId);
  barId.release();
  Id<Foo> fooId3(new Bar());
  Id<Bar> barId3;
  barId3 = fooId3;
  barId3.release();
  CPPUNIT_ASSERT(Foo::getCount() == count);
  return true;
}

bool IdTests::testConstId()
{
  Id<Foo> fooId(new Foo());
  const Id<const Foo> constFooId(fooId);
  CPPUNIT_ASSERT(constFooId->doConstFunc());
  fooId->increment();
  fooId.remove();
  return true;
}

class LabelTests {
public:
  static bool test(){
    EUROPA_runTest(testBasicAllocation);
    EUROPA_runTest(testElementCounting);
    EUROPA_runTest(testElementAccess);
    EUROPA_runTest(testComparisons);
    return true;
  }

private:
  static bool compare(const LabelStr& str1, const LabelStr& str2){
    return str1 == str2;
  }

  static bool testBasicAllocation(){
    LabelStr lbl1("");
    LabelStr lbl2("This is a char*");
    LabelStr lbl3(lbl2.toString());
    CPPUNIT_ASSERT(lbl3 == lbl2);
    std::string labelStr2("This is another char*");
    CPPUNIT_ASSERT(!LabelStr::isString(labelStr2));
    LabelStr lbl4(labelStr2);
    CPPUNIT_ASSERT(LabelStr::isString(labelStr2));
    CPPUNIT_ASSERT_MESSAGE(lbl4.toString() + " != " + lbl2.toString(), lbl4 != lbl2);

    edouble key = lbl2.getKey();
    LabelStr lbl5(key);
    CPPUNIT_ASSERT(lbl5 == lbl2);
    CPPUNIT_ASSERT(LabelStr::isString(key));
    CPPUNIT_ASSERT(!LabelStr::isString(1));

    CPPUNIT_ASSERT(compare(lbl3, lbl2));
    CPPUNIT_ASSERT(compare("This is another char*", "This is another char*"));
    return true;
  }

  static bool testElementCounting(){
    LabelStr lbl1("A 1B 1C 1D EFGH");
    CPPUNIT_ASSERT(lbl1.countElements("1") == 4);
    CPPUNIT_ASSERT(lbl1.countElements(" ") == 5);
    CPPUNIT_ASSERT(lbl1.countElements("B") == 2);
    CPPUNIT_ASSERT(lbl1.countElements(":") == 1);

    LabelStr lbl2("A:B:C:D:");
    CPPUNIT_ASSERT(lbl2.countElements(":") == 4);
    return true;
  }

  static bool testElementAccess(){
    LabelStr lbl1("A 1B 1C 1D EFGH");
    LabelStr first(lbl1.getElement(0, " "));
    CPPUNIT_ASSERT(first == LabelStr("A"));

    LabelStr last(lbl1.getElement(3, "1"));
    CPPUNIT_ASSERT(last == LabelStr("D EFGH"));
    return true;
  }

  static bool testComparisons(){
    LabelStr lbl1("A");
    LabelStr lbl2("G");
    LabelStr lbl3("B");
    LabelStr lbl4("B");
    CPPUNIT_ASSERT(lbl1 < lbl2);
    CPPUNIT_ASSERT(lbl2 > lbl4);
    CPPUNIT_ASSERT(lbl2 != lbl4);
    CPPUNIT_ASSERT(lbl4 == lbl3);

    LabelStr lbl5("ABCDEFGH");

    CPPUNIT_ASSERT(lbl5.contains("A"));
    CPPUNIT_ASSERT(lbl5.contains("H"));
    CPPUNIT_ASSERT(lbl5.contains("FG"));
    CPPUNIT_ASSERT(lbl5.contains(lbl5));
    CPPUNIT_ASSERT(!lbl5.contains("I"));
    return true;
  }
};

class EntityTest {
public:
  static bool test(){
    EUROPA_runTest(testReferenceCounting);
    return true;
  }

  class TestEntity: public Entity {
  public:
    TestEntity(): Entity() {}
    void handleDiscard(){}
  };

private:
  static bool testReferenceCounting(){
    // TestEntity* e1 = new TestEntity();
    // TestEntity* e2 = new TestEntity();
    // e1->discard();
    // CPPUNIT_ASSERT(e1->isDiscarded());
    // CPPUNIT_ASSERT(!e2->isDiscarded());
    // e2->incRefCount();
    // e2->decRefCount();
    // CPPUNIT_ASSERT(!e2->isDiscarded());
    // e2->decRefCount();
    // CPPUNIT_ASSERT(e2->isDiscarded());

    // CPPUNIT_ASSERT(Entity::garbageCollect() == 2);
    return true;
  }
};

//TODO: fill this out with more tests for XMLUtils
class XMLTest {
public:
  static bool test() {
    EUROPA_runTest(testInitXmlString);
    EUROPA_runTest(testExtractData);
    EUROPA_runTest(testInitXmlFile);
		EUROPA_runTest(testGetTextChild);
		EUROPA_runTest(testIsNumber);
    return true;
  }
private:
  static bool testExtractData() {
    std::string test("<Tag attribute=\"value\">");
    TiXmlElement* xml = initXml(test);
    CPPUNIT_ASSERT(xml != NULL);
    LabelStr value = extractData(*xml,"attribute");
    CPPUNIT_ASSERT(value == LabelStr("value"));
    return true;
  }

  static bool testInitXmlString() {
    std::string test("<Foo><Bar><Bing attr=\"baz\"/></Bar></Foo>");
    TiXmlElement* xml = initXml(test);
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Foo"));
    xml = xml->FirstChildElement();
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Bar"));
    xml = xml->FirstChildElement();
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Bing"));
    CPPUNIT_ASSERT(xml->Attribute("attr") != NULL);
    CPPUNIT_ASSERT(std::string(xml->Attribute("attr")) == std::string("baz"));
    return true;
  }

  static bool testInitXmlFile() {
	TiXmlElement* xml = initXml((getTestLoadLibraryPath() + "/XMLUtil-test.xml").c_str());
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("test"));
    xml = xml->FirstChildElement();
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Foo"));
    xml = xml->FirstChildElement();
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Bar"));
    xml = xml->FirstChildElement();
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Bing"));
    CPPUNIT_ASSERT(xml->Attribute("attr") != NULL);
    return true;
  }

  static bool testGetTextChild() {
    std::string test("<Foo>Ceci n'est pas un attribute</Foo>");
    TiXmlElement* xml = initXml(test);
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Foo"));
		CPPUNIT_ASSERT(std::string(getTextChild(*xml)) == std::string("Ceci n'est pas un attribute"));
    return true;
  }

  static bool testIsNumber() {
		CPPUNIT_ASSERT(isNumber("324.34532"));
		CPPUNIT_ASSERT(!isNumber("Foo"));
		double testNum = 0;
		// nice and prime:
		CPPUNIT_ASSERT(isNumber("31337",testNum));
		CPPUNIT_ASSERT(testNum == 31337);
		CPPUNIT_ASSERT(!isNumber("Bar"));
		CPPUNIT_ASSERT(testNum == 31337);
    return true;
  }
};

class XMLIOTest {
public:
	static bool test() {
    EUROPA_runTest(testInitIOXmlFile);
    EUROPA_runTest(testRead);
	EUROPA_runTest(testParse);
	EUROPA_runTest(testWrite);
    return true;
  }

private:

  static bool testInitIOXmlFile() {
	TiXmlElement* xml = initXml((getTestLoadLibraryPath() + "/config.xml").c_str());
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("EuropaConfig"));
    xml = xml->FirstChildElement();
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Component"));
    CPPUNIT_ASSERT(std::string(xml->Attribute("name")) == std::string("E"));
    xml = xml->FirstChildElement();
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Property"));
    CPPUNIT_ASSERT(std::string(xml->Attribute("name")) == std::string("loc"));
    CPPUNIT_ASSERT(std::string(xml->FirstChild()->ToText()->Value()) == std::string("here"));
    xml = xml->Parent()->NextSiblingElement();
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Component"));
    CPPUNIT_ASSERT(std::string(xml->Attribute("name")) == std::string("F"));
    xml = xml->FirstChildElement();
	CPPUNIT_ASSERT(xml != NULL);
	CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Property"));
	CPPUNIT_ASSERT(std::string(xml->Attribute("name")) == std::string("loc"));
	CPPUNIT_ASSERT(std::string(xml->FirstChild()->ToText()->Value()) == std::string("there"));
    xml = xml->NextSiblingElement();
    CPPUNIT_ASSERT(xml != NULL);
    CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Property"));
	CPPUNIT_ASSERT(std::string(xml->Attribute("name")) == std::string("type"));
	CPPUNIT_ASSERT(std::string(xml->FirstChild()->ToText()->Value()) == std::string("box"));
    return true;
  }

  static bool testRead() {
	EngineConfig* testEngine = new EngineConfig();
	testEngine->readFromXML((getTestLoadLibraryPath() + "/config.xml").c_str(),true);
	CPPUNIT_ASSERT(testEngine->readFromXML("<Foo><Bar><Bing attr=\"baz\"/></Bar></Foo>",false)==0);
	CPPUNIT_ASSERT(testEngine->getProperty("E.loc") == std::string("here"));
	CPPUNIT_ASSERT(testEngine->getProperty("F.loc") == std::string("there"));
	CPPUNIT_ASSERT(testEngine->getProperty("F.type") == std::string("box"));
	delete testEngine;
    return true;
  }

  //setup strings to be parsed and mapped node by node, use assert statements to check they were mapped properly
  static bool testParse() {
	EngineConfig* testEngine = new EngineConfig();
	CPPUNIT_ASSERT(testEngine != NULL);
	TiXmlElement* xml = initXml((getTestLoadLibraryPath() + "/config.xml").c_str());
	CPPUNIT_ASSERT(xml != NULL);
	testEngine->parseXML(xml);
	CPPUNIT_ASSERT(testEngine->getProperty("E.loc") == std::string("here"));
	CPPUNIT_ASSERT(testEngine->getProperty("F.loc") == std::string("there"));
	CPPUNIT_ASSERT(testEngine->getProperty("F.type") == std::string("box"));

    std::string test1 = "<EuropaConfig><Component name=\"A\"><Property name=\"FIX\">here</Property></Component><Component name=\"B\"><Property name=\"FIX\">there</Property></Component><Component name=\"C\"><Property name=\"FIX\">where</Property></Component></EuropaConfig>";
    std::string test2 = "<EuropaConfig><Component name=\"A\"><Property name=\"loc\">here</Property></Component><Component name=\"A\"><Property name=\"ked\">there</Property><Property name=\"up\">box</Property></Component></EuropaConfig>";
    std::string test3 = "<EuropaConfig><Component name=\"FIX\"><Property name=\"A\">1</Property><Property name=\"B\">2</Property><Property name=\"C\">3</Property></Component></EuropaConfig>";
    xml = initXml(test1);
    testEngine->parseXML(xml);
	CPPUNIT_ASSERT(testEngine->EngineConfig::getProperty("A.FIX") == std::string("here"));
	CPPUNIT_ASSERT(testEngine->EngineConfig::getProperty("B.FIX") == std::string("there"));
	CPPUNIT_ASSERT(testEngine->EngineConfig::getProperty("C.FIX") == std::string("where"));
	xml = initXml(test2);
	testEngine->parseXML(xml);
	CPPUNIT_ASSERT(testEngine->EngineConfig::getProperty("A.loc") == std::string("here"));
	CPPUNIT_ASSERT(testEngine->EngineConfig::getProperty("A.ked") == std::string("there"));
	CPPUNIT_ASSERT(testEngine->EngineConfig::getProperty("A.up") == std::string("box"));
	xml = initXml(test3);
	testEngine->parseXML(xml);
	CPPUNIT_ASSERT(testEngine->EngineConfig::getProperty("FIX.A") == std::string("1"));
	CPPUNIT_ASSERT(testEngine->EngineConfig::getProperty("FIX.B") == std::string("2"));
	CPPUNIT_ASSERT(testEngine->EngineConfig::getProperty("FIX.C") == std::string("3"));

	delete testEngine;
	return true;
  }

  static bool testWrite() {
	std::string writeTest(getTestLoadLibraryPath() + "/writeTest.xml");
    std::string test("<EuropaConfig><Component name=\"A\"><Property name=\"loc\">here</Property></Component></EuropaConfig>");
	EngineConfig* testEngine = new EngineConfig();
	TiXmlElement* xmlLoad = initXml(test);
	testEngine->parseXML(xmlLoad);
	testEngine->writeFromXML(writeTest.c_str());
	TiXmlElement* xml = initXml(writeTest.c_str());
	CPPUNIT_ASSERT(xml != NULL);
	CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("EuropaConfig"));
	xml = xml->FirstChildElement();
	CPPUNIT_ASSERT(xml != NULL);
	CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Component"));
	CPPUNIT_ASSERT(std::string(xml->Attribute("name")) == std::string("A"));
	xml = xml->FirstChildElement();
	CPPUNIT_ASSERT(xml != NULL);
	CPPUNIT_ASSERT(std::string(xml->Value()) == std::string("Property"));
	CPPUNIT_ASSERT(std::string(xml->Attribute("name")) == std::string("loc"));
	CPPUNIT_ASSERT(std::string(xml->FirstChild()->ToText()->Value()) == std::string("here"));
	delete testEngine;
    return true;
  }

};

class NumberTest {
public:
  static bool test() {
    EUROPA_runTest(testSize);
    EUROPA_runTest(testEint);
    EUROPA_runTest(testEintInfinity);
    EUROPA_runTest(testEdouble);
    EUROPA_runTest(testEdoubleInfinity);
    return true;
  }
private:

  static bool testSize() {
    CPPUNIT_ASSERT_EQUAL(sizeof(eint::basis_type), sizeof(eint));
    CPPUNIT_ASSERT_EQUAL(sizeof(edouble::basis_type), sizeof(edouble));
    return true;
  }
  static bool testEint() {
    EUROPA::eint e(3);
#ifdef E2_LONG_INT
    long i = 3;
#else
    int i = 3;
#endif

    CPPUNIT_ASSERT(e == i);
    CPPUNIT_ASSERT((+e) == (+i));
    CPPUNIT_ASSERT((-e) == (-i));
    CPPUNIT_ASSERT((!e) == (!i));
    CPPUNIT_ASSERT((++e) == (++i));
    CPPUNIT_ASSERT((e++) == (i++));
    CPPUNIT_ASSERT(e == i);
    CPPUNIT_ASSERT((--e) == (--i));
    CPPUNIT_ASSERT((e--) == (i--));
    CPPUNIT_ASSERT(e == i);
    CPPUNIT_ASSERT((e + 1) == (i + 1));
    CPPUNIT_ASSERT((e - 1) == (i - 1));
    CPPUNIT_ASSERT((e * 2) == (i * 2));
    CPPUNIT_ASSERT((e / 2) == (i / 2));
    CPPUNIT_ASSERT((e % 2) == (i % 2));
    CPPUNIT_ASSERT((e += 3) == (i += 3));
    CPPUNIT_ASSERT(e == i);
    CPPUNIT_ASSERT((e -= 2) == (i -= 2));
    CPPUNIT_ASSERT(e == i);
    CPPUNIT_ASSERT((e *= 2) == (i *= 2));
    CPPUNIT_ASSERT(e == i);
    CPPUNIT_ASSERT((e /= 2) == (i /= 2));
    CPPUNIT_ASSERT(e == i);
    CPPUNIT_ASSERT((e %= 3) == (i %= 3));
    CPPUNIT_ASSERT(e == i);
    e = 50;
    i = 50;
    CPPUNIT_ASSERT((e < 900) == (i < 900));
    CPPUNIT_ASSERT((e < -900) == (i < -900));
    CPPUNIT_ASSERT((e <= 900) == (i <= 900));
    CPPUNIT_ASSERT((e <= -900) == (i <= -900));
    CPPUNIT_ASSERT((e <= 50));
    CPPUNIT_ASSERT(e == 50);
    CPPUNIT_ASSERT((e >= 10) && (i >= 10));
    CPPUNIT_ASSERT(e >= 50);
    CPPUNIT_ASSERT((e > 10) && (i > 10));
    CPPUNIT_ASSERT(e != 10);

    //there's some weird compilation thing with creating an eint out of a size_t
    std::vector<int> foo;
    foo.push_back(1);
    eint f(foo.size());
    CPPUNIT_ASSERT(f == 1);

    return true;
  }
  static bool testEdouble() {
    edouble e(3.12);
    double d(3.12);

    CPPUNIT_ASSERT(e == d);
    CPPUNIT_ASSERT((+e) == (+d));
    CPPUNIT_ASSERT((-e) == (-d));
    CPPUNIT_ASSERT((++e) == (++d));
    CPPUNIT_ASSERT((e++) == (d++));
    CPPUNIT_ASSERT(e == d);
    CPPUNIT_ASSERT((--e) == (--d));
    CPPUNIT_ASSERT((e--) == (d--));
    CPPUNIT_ASSERT(e == d);
    CPPUNIT_ASSERT((e + 1) == (d + 1));
    CPPUNIT_ASSERT((e - 1) == (d - 1));
    CPPUNIT_ASSERT((e * 2) == (d * 2));
    CPPUNIT_ASSERT((e / 2) == (d / 2));
    CPPUNIT_ASSERT((e += 3) == (d += 3));
    CPPUNIT_ASSERT(e == d);
    CPPUNIT_ASSERT((e -= 2) == (d -= 2));
    CPPUNIT_ASSERT(e == d);
    CPPUNIT_ASSERT((e *= 2) == (d *= 2));
    CPPUNIT_ASSERT(e == d);
    CPPUNIT_ASSERT((e /= 2) == (d /= 2));
    CPPUNIT_ASSERT(e == d);
    e = 50.9;
    d = 50.9;
    CPPUNIT_ASSERT((e < 900) == (d < 900));
    CPPUNIT_ASSERT((e < -900) == (d < -900));
    CPPUNIT_ASSERT((e <= 900) == (d <= 900));
    CPPUNIT_ASSERT((e <= -900) == (d <= -900));
    CPPUNIT_ASSERT((e <= 50.9));
    CPPUNIT_ASSERT(e == 50.9);
    CPPUNIT_ASSERT((e >= 10) && (d >= 10));
    CPPUNIT_ASSERT(e >= 50.9);
    CPPUNIT_ASSERT((e > 10) && (d > 10));
    CPPUNIT_ASSERT(e != 10);
    CPPUNIT_ASSERT((e < 900.) == (d < 900.));
    CPPUNIT_ASSERT((e < -900.) == (d < -900.));
    CPPUNIT_ASSERT((e <= 900.) == (d <= 900.));
    CPPUNIT_ASSERT((e <= -900.) == (d <= -900.));
    CPPUNIT_ASSERT((e <= 50.9));
    CPPUNIT_ASSERT(e == 50.9);
    CPPUNIT_ASSERT((e >= 10.0) && (d >= 10.0));
    CPPUNIT_ASSERT(e >= 50.9);
    CPPUNIT_ASSERT((e > 10.) && (d > 10.));
    CPPUNIT_ASSERT(e != 10.);

    return true;
  }
  
  static bool testEintInfinity() {
    eint pinf(std::numeric_limits<eint>::infinity());
    eint minf(std::numeric_limits<eint>::minus_infinity());

    CPPUNIT_ASSERT((minf) == minf);
    CPPUNIT_ASSERT((minf - 1) == minf);
    CPPUNIT_ASSERT((minf + 1) == minf + 1);
    CPPUNIT_ASSERT((pinf + 1) == pinf);
    CPPUNIT_ASSERT((pinf - 1) == pinf);
    CPPUNIT_ASSERT((pinf - 1) == pinf - 1);
    CPPUNIT_ASSERT((pinf + pinf) == pinf);
    CPPUNIT_ASSERT((pinf - pinf) == 0);
    CPPUNIT_ASSERT((minf + minf) == minf);
    CPPUNIT_ASSERT((minf + pinf) == 0);
    return true;
  }

  static bool testEdoubleInfinity() {
    edouble pinf(std::numeric_limits<edouble>::infinity());
    edouble minf(std::numeric_limits<edouble>::minus_infinity());

    CPPUNIT_ASSERT((minf) == minf);
    CPPUNIT_ASSERT((minf - 1) == minf);
    CPPUNIT_ASSERT((minf + 1) == minf + 1);
    CPPUNIT_ASSERT((pinf + 1) == pinf);
    CPPUNIT_ASSERT((pinf - 1) == pinf - 1);
    CPPUNIT_ASSERT((pinf + pinf) == pinf);
    CPPUNIT_ASSERT((pinf - pinf) == 0);
    CPPUNIT_ASSERT((minf + minf) == minf);
    CPPUNIT_ASSERT((minf + pinf) == 0);
    return true;
  }
};

void UtilModuleTests::errorTests()
{
	ErrorTest::test();
}

void UtilModuleTests::debugTests()
{
	DebugTest::test();
}
void UtilModuleTests::idTests()
{
	IdTests::test();
}
void UtilModuleTests::labelTests()
{
	LabelTests::test();
}

void UtilModuleTests::entityTests()
{
	EntityTest::test();
}

void UtilModuleTests::xmlTests()
{
	XMLTest::test();
}

void UtilModuleTests::numberTests() {
  NumberTest::test();
}

void UtilModuleTests::xmlIOTests()
{
	XMLIOTest::test();
}
