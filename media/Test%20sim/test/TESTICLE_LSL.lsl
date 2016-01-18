integer verbosity = 1;

// All those that fail at compile time, or cause crashes, are commented out

integer testsPassed = 0;
integer testsFailed = 0;


testPassed(string description, string actual, string expected)
{
    ++testsPassed;
    llSay(0, description);
}

testFailed(string description, string actual, string expected)
{
    ++testsFailed;
//    llSay(0, "FAILED!: " + description + " (" + actual + " expected " + expected + ")");
llSay(0, "FAILED!: " + description);
}

ensureTrue(string description, integer actual)
{
    if(actual)
    {
        testPassed(description, (string) actual, (string) TRUE);
    }
    else
    {
        testFailed(description, (string) actual, (string) TRUE);
    }
}

ensureFalse(string description, integer actual)
{
    if(actual)
    {
        testFailed(description, (string) actual, (string) FALSE);
    }
    else
    {
        testPassed(description, (string) actual, (string) FALSE);
    }
}

ensureIntegerEqual(string description, integer actual, integer expected)
{
    if(actual == expected)
    {
        testPassed(description, (string) actual, (string) expected);
    }
    else
    {
        testFailed(description, (string) actual, (string) expected);
    }
}

ensureFloatEqual(string description, float actual, float expected)
{
    if(actual == expected)
    {
        testPassed(description, (string) actual, (string) expected);
    }
    else
    {
        testFailed(description, (string) actual, (string) expected);
    }
}

ensureStringEqual(string description, string actual, string expected)
{
    if(actual == expected)
    {
        testPassed(description, (string) actual, (string) expected);
    }
    else
    {
        testFailed(description, (string) actual, (string) expected);
    }
}

ensureVectorEqual(string description, vector actual, vector expected)
{
    if(actual == expected)
    {
        testPassed(description, (string) actual, (string) expected);
    }
    else
    {
        testFailed(description, (string) actual, (string) expected);
    }
}

ensureRotationEqual(string description, rotation actual, rotation expected)
{
    if(actual == expected)
    {
        testPassed(description, (string) actual, (string) expected);
    }
    else
    {
        testFailed(description, (string) actual, (string) expected);
    }
}

ensureListEqual(string description, list actual, list expected)
{
    if(actual == expected)
    {
        testPassed(description, (string) actual, (string) expected);
    }
    else
    {
        testFailed(description, (string) actual, (string) expected);
    }
}

integer gInteger = 5;
float gFloat = 1.5;
string gString = "foo";
vector gVector = <1, 2, 3>;
rotation gRot = <1, 2, 3, 4>;

integer testReturn()
{
    return 1;
}

integer testParameters(integer param)
{
    param = param + 1;
    return param;
}

integer testRecursion(integer param)
{
    if(param <= 0)
    {
        return 0;
    }
    else
    {
        return testRecursion(param - 1);
    }
}




default
{
  state_entry()
  {
    integer success = 1;
    string result = "";

    // reset globals  
    gInteger = 5;
    gFloat = 1.5;
    gString = "foo";
//    gVector = <1, 2, 3>;
    gRot = <1, 2, 3, 4>;
    testsPassed = 0;
    testsFailed = 0;


    //
    // Put your tests here.
    //

    // truth
    ensureIntegerEqual("TRUE", TRUE, TRUE);
    ensureIntegerEqual("FALSE", FALSE, FALSE);

    // equality
    ensureIntegerEqual("(TRUE == TRUE)", (TRUE == TRUE), TRUE);
    ensureIntegerEqual("(TRUE == FALSE)", (TRUE == FALSE), FALSE);
    ensureIntegerEqual("(TRUE == FALSE)", (TRUE == FALSE), FALSE);
    ensureIntegerEqual("(FALSE == FALSE)", (FALSE == FALSE), TRUE);

    // inequality
    ensureIntegerEqual("(TRUE != TRUE)", (TRUE != TRUE), FALSE);
    ensureIntegerEqual("(TRUE != FALSE)", (TRUE != FALSE), TRUE);
    ensureIntegerEqual("(TRUE != FALSE)", (TRUE != FALSE), TRUE);
    ensureIntegerEqual("(FALSE != FALSE)", (FALSE != FALSE), FALSE);

    // and
    ensureIntegerEqual("(TRUE && TRUE)", (TRUE && TRUE), TRUE);
    ensureIntegerEqual("(TRUE && FALSE)", (TRUE && FALSE), FALSE);
    ensureIntegerEqual("(FALSE && TRUE)", (FALSE && TRUE), FALSE);
    ensureIntegerEqual("(FALSE && FALSE)", (FALSE && FALSE), FALSE);

    // or
    ensureIntegerEqual("(TRUE || TRUE)", (TRUE || TRUE), TRUE);
    ensureIntegerEqual("(TRUE || FALSE)", (TRUE || FALSE), TRUE);
    ensureIntegerEqual("(FALSE || TRUE)", (FALSE || TRUE), TRUE);
    ensureIntegerEqual("(FALSE || FALSE)", (FALSE || FALSE), FALSE);

    // not
    ensureIntegerEqual("(! TRUE)", (! TRUE), FALSE);
    ensureIntegerEqual("(! FALSE)", (! FALSE), TRUE);

    // greater than
    ensureIntegerEqual("(1 > 0)", (1 > 0), TRUE);
    ensureIntegerEqual("(0 > 1)", (0 > 1), FALSE);
    ensureIntegerEqual("(1 > 1)", (1 > 1), FALSE);

    // less than
    ensureIntegerEqual("(0 < 1)", (0 < 1), TRUE);
    ensureIntegerEqual("(1 < 0)", (1 < 0), FALSE);
    ensureIntegerEqual("(1 < 1)", (1 < 1), FALSE);

    // greater than or equal
    ensureIntegerEqual("(1 >= 0)", (1 >= 0), TRUE);
    ensureIntegerEqual("(0 >= 1)", (0 >= 1), FALSE);
    ensureIntegerEqual("(1 >= 1)", (1 >= 1), TRUE);

    // tess than or equal
    ensureIntegerEqual("(0 <= 1)", (0 <= 1), TRUE);
    ensureIntegerEqual("(1 <= 0)", (1 <= 0), FALSE);
    ensureIntegerEqual("(1 <= 1)", (1 <= 1), TRUE);

    // bitwise and
    ensureIntegerEqual("(10 & 25)", (10 & 25), 8);

    // bitwise or
    ensureIntegerEqual("(10 | 25)", (10 | 25), 27);

    // bitwise not
//    ensureIntegerEqual("~10", ~10, -11);

    // xor
//    ensureIntegerEqual("(10 ^ 25)", (10 ^ 25), 19);

    // right shift
    ensureIntegerEqual("(523 >> 2)", (523 >> 2), 130);

    // left shift
    ensureIntegerEqual("(523 << 2)", (523 << 2), 2092);

    // addition
    ensureIntegerEqual("(1 + 1)", (1 + 1), 2);
    ensureFloatEqual("(1 + 1.1)", (1 + 1.1), 2.1);
    ensureFloatEqual("(1.1 + 1)", (1.1 + 1), 2.1);
    ensureFloatEqual("(1.1 + 1.1)", (1.1 + 1.1), 2.2);
    ensureStringEqual("\"foo\" + \"bar\"", "foo" + "bar", "foobar");
//    ensureVectorEqual("(<1.1, 2.2, 3.3> + <4.4, 5.5, 6.6>)", (<1.1, 2.2, 3.3> + <4.4, 5.5, 6.6>), <5.5, 7.7, 9.9>);
//    ensureRotationEqual("(<1.1, 2.2, 3.3, 4.4> + <4.4, 5.5, 6.6, 3.3>)", (<1.1, 2.2, 3.3, 4.4> + <4.4, 5.5, 6.6, 3.3>), <5.5, 7.7, 9.9, 7.7>);
//    ensureListEqual("([1] + 2)", ([1] + 2), [1,2]);
//    ensureListEqual("([] + 1.5)", ([] + 1.5), [1.5]);
//    ensureListEqual("([\"foo\"] + \"bar\")", (["foo"] + "bar"), ["foo", "bar"]);
//    ensureListEqual("([] + <1,2,3>)", ([] + <1,2,3>), [<1,2,3>]);
//    ensureListEqual("([] + <1,2,3,4>)", ([] + <1,2,3,4>), [<1,2,3,4>]);

    // subtraction
    ensureIntegerEqual("(1 - 1)", (1 - 1), 0);
    ensureFloatEqual("(1 - 0.5)", (1 - 0.5), 0.5);
    ensureFloatEqual("(1.5 - 1)", (1.5 - 1), 0.5);
    ensureFloatEqual("(2.2 - 1.1)", (2.2 - 1.1), 1.1);
//    ensureVectorEqual("(<1.5, 2.5, 3.5> - <4.5, 5.5, 6.5>)", (<1.5, 2.5, 3.5> - <4.5, 5.5, 6.5>), <-3.0, -3.0, -3.0>);
//    ensureRotationEqual("(<1.5, 2.5, 3.5, 4.5> - <4.5, 5.5, 6.5, 7.5>)", (<1.5, 2.5, 3.5, 4.5> - <4.5, 5.5, 6.5, 7.5>), <-3.0, -3.0, -3.0, -3.0>);

    // multiplication
    ensureIntegerEqual("(2 * 3)", (2 * 3), 6);
    ensureFloatEqual("(2 * 3.5)", (2 * 3.5), 7.0);
    ensureFloatEqual("(2.5 * 3)", (2.5 * 3), 7.5);
    ensureFloatEqual("(2.5 * 3.5)", (2.5 * 3.5), 8.75);
//    ensureVectorEqual("(<1.1, 2.2, 3.3> * 2)", (<1.1, 2.2, 3.3> * 2), <2.2, 4.4, 6.6>);
//    ensureVectorEqual("(<2.2, 4.4, 6.6> * 2.0)", (<2.2, 4.4, 6.6> * 2.0), <4.4, 8.8, 13.2>);
//    ensureFloatEqual("(<1.0, 2.0, 3.0> * <4.0, 5.0, 6.0>)", (<1.0, 2.0, 3.0> * <4.0, 5.0, 6.0>), 32.0);

    // division
    ensureIntegerEqual("(2 / 2)", (2 / 2), 1);
    ensureFloatEqual("(2.2 / 2)", (2.2 / 2), 1.1);
    ensureFloatEqual("(3 / 1.5)", (3 / 1.5), 2.0);
    ensureFloatEqual("(2.2 / 2.0)", (2.2 / 2.0), 1.1);
//    ensureVectorEqual("(<1.0, 2.0, 3.0> / 2)", (<1.0, 2.0, 3.0> / 2), <0.5, 1.0, 1.5>);
//    ensureVectorEqual("(<3.0, 6.0, 9.0> / 1.5)", (<3.0, 6.0, 9.0> / 1.5), <2.0, 4.0, 6.0>);

    // modulo
    ensureIntegerEqual("(3 % 1)", (3 % 1), 0);
//    ensureVectorEqual("(<1.0, 2.0, 3.0> % <4.0, 5.0, 6.0>)", (<1.0, 2.0, 3.0> % <4.0, 5.0, 6.0>), <-3.0, 6.0, -3.0>);

    // assignment
    integer i = 1;
    ensureIntegerEqual("i = 1;", i, 1);

    // addition assignment
    i = 1;
    i += 1;
    ensureIntegerEqual("i = 1; i += 1;", i, 2);

    // subtraction assignment
    i = 1;
    i -= 1;
    ensureIntegerEqual("i = 1; i -= 1;", i, 0);

    // multiplication assignment
    i = 2;
    i *= 2;
    ensureIntegerEqual("i = 2; i *= 2;", i, 4);

    // division assignment
    i = 2;
    i /= 2;
    ensureIntegerEqual("i = 2; i /= 2;", i, 1);

    // modulo assignment
    i = 3;
    i %= 1;
    ensureIntegerEqual("i = 3; i %= 1;", i, 0);

    // post increment.
    i = 1;
    ensureIntegerEqual("i = 1; (i == 2) && (i++ == 1)", (i == 2) && (i++ == 1), TRUE);

    // pre increment.
    i = 1;
    ensureIntegerEqual("i = 1; (i == 2) && (++i == 2)", (i == 2) && (++i == 2), TRUE);

    // post decrement.
    i = 1;
    ensureIntegerEqual("i = 1; (i == 0) && (i-- == 1)", (i == 0) && (i-- == 1), TRUE);

    // pre decrement.
    i = 1;
    ensureIntegerEqual("i = 1; (i == 0) && (--i == 0)", (i == 0) && (--i == 0), TRUE);

    // casting
    ensureFloatEqual("((float)2)", ((float)2), 2.0);
    ensureStringEqual("((string)2)", ((string)2), "2");
    ensureIntegerEqual("((integer) 1.5)", ((integer) 1.5), 1);
    ensureStringEqual("((string) 1.5)", ((string) 1.5), "1.500000");
    ensureIntegerEqual("((integer) \"0xF\")", ((integer) "0xF"), 15);
    ensureIntegerEqual("((integer) \"2\")", ((integer) "2"), 2);
    ensureFloatEqual("((float) \"1.5\")", ((float) "1.5"), 1.5);
//    ensureVectorEqual("((vector) \"<1,2,3>\")", ((vector) "<1,2,3>"), <1,2,3>);
//    ensureRotationEqual("((rotation) \"<1,2,3,4>\")", ((rotation) "<1,2,3,4>"), <1,2,3,4>);
    ensureStringEqual("((string) <1,2,3>)", ((string) <1,2,3>), "<1.00000, 2.00000, 3.00000>");
    ensureStringEqual("((string) <1,2,3,4>)", ((string) <1,2,3,4>), "<1.00000, 2.00000, 3.00000, 4.00000>");
    ensureStringEqual("((string) [1,2.5,<1,2,3>])", ((string) [1,2.5,<1,2,3>]), "12.500000<1.000000, 2.000000, 3.000000>");

    // while
    i = 0;
//    while(i < 10) ++i;
    ensureIntegerEqual("i = 0; while(i < 10) ++i", i, 10);

    // do while
    i = 0;
//    do {++i;} while(i < 10);
    ensureIntegerEqual("i = 0; do {++i;} while(i < 10);", i, 10);

    // for
//    for(i = 0; i < 10; ++i);
    ensureIntegerEqual("for(i = 0; i < 10; ++i);", i, 10);

    // jump
    i = 1;
//    jump SkipAssign;
    i = 2;
//    @SkipAssign;
    ensureIntegerEqual("i = 1; jump SkipAssign; i = 2; @SkipAssign;", i, 1);

    // return
    ensureIntegerEqual("testReturn()", testReturn(), 1);

    // parameters
    ensureIntegerEqual("testParameters(1)", testParameters(1), 2);

    // variable parameters
    i = 1;
    ensureIntegerEqual("i = 1; testParameters(i)", testParameters(i), 2);

    // recursion
    ensureIntegerEqual("testRecursion(10)", testRecursion(10), 0);

    // globals
    ensureIntegerEqual("gInteger", gInteger, 5);
    ensureFloatEqual("gFloat", gFloat, 1.5);
    ensureStringEqual("gString", gString, "foo");
//    ensureVectorEqual("gVector", gVector, <1, 2, 3>);
//    ensureRotationEqual("gRot", gRot, <1, 2, 3, 4>);

    // global assignment
    gInteger = 1;
    ensureIntegerEqual("gInteger = 1", gInteger, 1);

    gFloat = 0.5;
    ensureFloatEqual("gFloat = 0.5", gFloat, 0.5);

    gString = "bar";
    ensureStringEqual("gString = \"bar\"", gString, "bar");

//    gVector = <3,3,3>;
//    ensureVectorEqual("gVector = <3,3,3>", gVector, <3,3,3>);

//    gRot = <3,3,3,3>;
//    ensureRotationEqual("gRot = <3,3,3,3>", gRot, <3,3,3,3>);

    // vector accessor
    vector v;
    v.x = 3;
    ensureFloatEqual("v.x", v.x, 3);

    // rotation accessor
    rotation q;
    q.s = 5;
    ensureFloatEqual("q.s", q.s, 5);

    // global vector accessor
    gVector.y = 17.5;
    ensureFloatEqual("gVector.y = 17.5", gVector.y, 17.5);

    // global rotation accessor
    gRot.z = 19.5;
    ensureFloatEqual("gRot.z = 19.5", gRot.z, 19.5);

    // list equality
    list l = (list) 5;
    list l2 = (list) 5;
    ensureListEqual("list l = (list) 5; list l2 = (list) 5", l, l2);
    ensureListEqual("list l = (list) 5", l, [5]);
    ensureListEqual("[1.5, 6, <1,2,3>, <1,2,3,4>]", [1.5, 6, <1,2,3>, <1,2,3,4>], [1.5, 6, <1,2,3>, <1,2,3,4>]);


    if (testsFailed > 0)
    {
      if (0 == verbosity)
        result = result + "Shhh.";
      if (1 == verbosity)
        result = result + "Failed the testicle.";
      if (2 == verbosity)
        result = result + "The script called '" + llGetScriptName() + ".lsl' has failed " + (string) testsFailed  + " of it's " + (string) (testsFailed + testsPassed) + " tests.";
      result = result + " - FAILURE!!!";
    }
    else
    {
      if (0 == verbosity)
        result = result + "Shhh.";
      if (1 == verbosity)
        result = result + "Passed the testicle.";
      if (2 == verbosity)
        result = result + "The script called '" + llGetScriptName() + ".lsl' has passed all " + (string) (testsFailed + testsPassed) + " of it's tests.";
      result = result + " - SUCCESS";
    }

    llSay(0, llGetScriptName() + ": " + result);
  }

  on_rez(integer start_param)
  {
    llResetScript();
  }
}
