
default
{
    state_entry()
    {
	llSay(0, /* */ 4 /* c */ + /* c0 */ 2 /* c1 */ * /* c2 */ 10 /* c3 */ + /* c4 */
	3 /* c5 */ * /* c6 */ ( /* c7 */ 5 /* c8 */ + /* c9 */ 1 /* cA */ ) /* cB */) ; /* cE */
	// Some more arithmetic -
	llSay(0, 1+1);
	llSay(0, 20.5 + 20 + 1.5);
	llDialog(llGetOwner(), "This is an llDialog() window.", ["1", "2", "3", "4", "5", "6", "7", "8"], 0);
    }
}

// This is the end my friend.
