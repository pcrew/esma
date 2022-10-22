Somtimes we want to create state machine from scratch.

```
states {
	state_1: __special_function_1__;
	state_2: __special_function_2__;
	state_3: __special_function_3__;
	state_4: __special_function_4__;
};

trans {
	init -> state_1: 0;
	init -> state_2: 1;
	init -> state_3: 2;

	state_1 -> state_2: 0;
	state_1 -> state_4: 1;
	state_1 -> fini: 2;

	state_2 -> self: sign_0: SIGUSR1;
	state_2 -> state_3: 1;

	state_3 -> state_4: 0;
	
	state_4 -> fini: 0;
};
```
