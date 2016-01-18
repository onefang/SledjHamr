integer verbosity = 1;


default
{
  state_entry()
  {
    integer success = 1;
    string result = "";


    //
    // Put your tests here.
    //


    if (success)
    {
      if (0 == verbosity)
        result = result + "Shhh.";
      if (1 == verbosity)
        result = result + "Passed the testicle.";
      if (2 == verbosity)
        result = result + "The script called '" + llGetScriptName() + ".lsl' has passed all of it's tests, not that it has any.";
      result = result + " - SUCCESS";
    }
    else
    {
      if (0 == verbosity)
        result = result + "Shhh.";
      if (1 == verbosity)
        result = result + "Failed the testicle.";
      if (2 == verbosity)
        result = result + "The script called '" + llGetScriptName() + ".lsl' has failed all of it's tests, not that it has any.";
      result = result + " - FAILURE!!!";
    }

    llSay(0, llGetScriptName() + ": " + result);
  }

  on_rez(integer start_param)
  {
    llResetScript();
  }
}
