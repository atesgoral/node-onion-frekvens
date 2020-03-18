#include <main.h>

void initGpioSetup (gpioSetup* obj)
{
	obj->pinNumber	= -1;
	obj->pinValue	= 0;
	obj->pinDir		= 0;

	obj->bPwm		= 0;
	obj->pwmFreq	= 0;
	obj->pwmDuty	= 0;

	obj->verbose 	= FASTGPIO_DEFAULT_VERBOSITY;
	obj->debug 		= FASTGPIO_DEFAULT_DEBUG;

	obj->cmdString 	= new char[255];
}

void usage(const char* progName) {
	printf("Usage:\n");
	printf("\t%s set-input <gpio>\n", progName);
	printf("\t%s set-output <gpio>\n", progName);
	printf("\t%s get-direction <gpio>\n", progName);
	printf("\t%s read <gpio>\n", progName);
	printf("\t%s set <gpio> <value: 0 or 1>\n", progName);
	printf("\t%s pwm <gpio> <freq in Hz> <duty cycle percentage>\n", progName);
	printf("\t%s pulses <gpio> <path_pulses_file> <repeats>\n",progName);
	printf("\n");
}

void print(int verbosity, char* cmd, int pin, char* val)
{
	if 	(	verbosity != FASTGPIO_VERBOSITY_QUIET &&
			verbosity != FASTGPIO_VERBOSITY_JSON
		)
	{
		printf(FASTGPIO_STDOUT_STRING, cmd, pin, val);
	}
	else if ( verbosity == FASTGPIO_VERBOSITY_JSON ) {
		printf(FASTGPIO_JSON_STRING, cmd, pin, val);
	}
}

int parseArguments(const char* progName, int argc, char* argv[], gpioSetup *setup)
{
	// check for the correct number of arguments
	if ( argc < 2 )
	{
		return EXIT_FAILURE;
	}


	// parse the command line arguments
	//	arg1 - command: read, set
	// 	arg2 - gpio pin number
	// 	arg3 - value to write in case of set
	if (strncmp(argv[0], "set-input", strlen("set-input") ) == 0 )	{
		setup->cmd 		= GPIO_CMD_SET_DIRECTION;
		setup->pinDir 	= 0;
		strcpy(setup->cmdString, FASTGPIO_CMD_STRING_SET_DIR);
	}
	else if (strncmp(argv[0], "set-output", strlen("set-output") ) == 0 )	{
		setup->cmd 		= GPIO_CMD_SET_DIRECTION;
		setup->pinDir 	= 1;
		strcpy(setup->cmdString, FASTGPIO_CMD_STRING_SET_DIR);
	}
	else if (strncmp(argv[0], "get-direction", strlen("get-direction") ) == 0 )	{
		setup->cmd 		= GPIO_CMD_GET_DIRECTION;
		strcpy(setup->cmdString, FASTGPIO_CMD_STRING_GET_DIR);
	}
	else if (strncmp(argv[0], "set", strlen("set") ) == 0 )	{
		setup->cmd 		= GPIO_CMD_SET;
		strcpy(setup->cmdString, FASTGPIO_CMD_STRING_SET);

		// get the write value
		if ( argc == 3 ) {
			setup->pinValue	= atoi(argv[2]);
		}
		else {
			return EXIT_FAILURE;
		}
	}
	else if (strncmp(argv[0], "read", strlen("read") ) == 0 )	{
		setup->cmd 		= GPIO_CMD_READ;
		strcpy(setup->cmdString, FASTGPIO_CMD_STRING_READ);
	}
	else if (strncmp(argv[0], "pulses", strlen("pulses") ) == 0 )	{
		setup->cmd 		= GPIO_CMD_PULSES;
		strcpy(setup->cmdString, FASTGPIO_CMD_STRING_PULSES);
		// get the path to the pulses file and repeat number
		setup->pathPulsesFile = argv[2];
		setup->repeats = atoi(argv[3]);
	}

	else if (strncmp(argv[0], "pwm", strlen("pwm") ) == 0 )	{
		setup->cmd 	= GPIO_CMD_PWM;
		strcpy(setup->cmdString, FASTGPIO_CMD_STRING_PWM);

		// get the freq and duty values
		if ( argc == 4 ) {
			setup->pwmFreq	= atoi(argv[2]);
			setup->pwmDuty	= atoi(argv[3]);
		}
		else {
			return EXIT_FAILURE;
		}
	}
	else {
		return EXIT_FAILURE;
	}

	// get the pin number
	setup->pinNumber 	= atoi(argv[1]);

	return EXIT_SUCCESS;
}

void frekvensRun(gpioSetup* setup) {
	const int PIN_R = 17;
	const int PIN_G = 16;
	const int PIN_B = 15;

	const int PIN_LATCH = 2;
	const int PIN_CLOCK = 1;
	const int PIN_DATA = 0;

	FastGpio *gpioObj;

	if (strcmp(DEVICE_TYPE, "ramips") == 0) {
		gpioObj = new FastGpioOmega2();
	} else {
		gpioObj = new FastGpioOmega();
	}

	gpioObj->SetVerbosity(setup->verbose == FASTGPIO_VERBOSITY_ALL ? 1 : 0);
	gpioObj->SetDebugMode(setup->debug);

	// gpioObj->SetDirection(PIN_R, 1); // set to output
	// gpioObj->SetDirection(PIN_G, 1); // set to output
	// gpioObj->SetDirection(PIN_B, 1); // set to output

	gpioObj->SetDirection(PIN_LATCH, 1); // set to output
	gpioObj->SetDirection(PIN_CLOCK, 1); // set to output
	gpioObj->SetDirection(PIN_DATA, 1); // set to output

	// gpioObj->Set(PIN_R, 1);
	// gpioObj->Set(PIN_G, 1);
	// gpioObj->Set(PIN_B, 1);

	gpioObj->Set(PIN_LATCH, 0);
	gpioObj->Set(PIN_CLOCK, 0);
	gpioObj->Set(PIN_DATA, 0);

	int pixels[] = {};

	int f = 0;

	while (1) {
		for (int i = 0; i < 16 * 16; i++) {
			gpioObj->Set(PIN_DATA, pixels[i]);

			gpioObj->Set(PIN_CLOCK, 1);
			usleep(1);
			gpioObj->Set(PIN_CLOCK, 0);
			usleep(1);
		}

		gpioObj->Set(PIN_LATCH, 1);
		usleep(1);
		gpioObj->Set(PIN_LATCH, 0);
		usleep(1);

		f++;

		// gpioObj->Set(PIN_R, f & 1);
		// gpioObj->Set(PIN_G, f >> 1 & 1);
		// gpioObj->Set(PIN_B, f >> 2 & 1);

		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 16; x++) {
				pixels[((x & 8) << 4) + (x & 7) + (y << 3)] = (x + f) & y ? 1 : 0;
			}
		}

		usleep(1000 * 1000 / 60);
	}
}

// function to run gpio commands
int gpioRun(gpioSetup* setup)
{
	int status	= EXIT_SUCCESS;
	FastGpio	* gpioObj;
		// object setup
	if (strcmp(DEVICE_TYPE, "ramips") == 0) {
		gpioObj = new FastGpioOmega2();
	} else {
		gpioObj = new FastGpioOmega();
	}
	char 		valString[255];
	// object setup
	gpioObj->SetVerbosity(setup->verbose == FASTGPIO_VERBOSITY_ALL ? 1 : 0);
	gpioObj->SetDebugMode(setup->debug);

	// object operations
	switch (setup->cmd) {
		case GPIO_CMD_SET:
			gpioObj->SetDirection(setup->pinNumber, 1); // set to output
			gpioObj->Set(setup->pinNumber, setup->pinValue);

			strcpy(valString, (setup->pinValue == 1 ? "1" : "0") );
			break;

		case GPIO_CMD_READ:
			gpioObj->Read(setup->pinNumber, setup->pinValue);
			strcpy(valString, (setup->pinValue == 1 ? "1" : "0") );
			break;

		case GPIO_CMD_SET_DIRECTION:
			gpioObj->SetDirection(setup->pinNumber, setup->pinDir); // set pin direction
			strcpy(valString, (setup->pinDir == 1 ? "output" : "input") );
			break;

		case GPIO_CMD_GET_DIRECTION:
			gpioObj->GetDirection(setup->pinNumber, setup->pinDir); // find pin direction
			strcpy(valString, (setup->pinDir == 1 ? "output" : "input") );
			break;
		case GPIO_CMD_PULSES:
			pulseGpio(gpioObj,setup->pinNumber,setup->pathPulsesFile,setup->repeats);
			break;

		default:
			status = EXIT_FAILURE;
			break;
	}

	if (status != EXIT_FAILURE) {
		print(setup->verbose, setup->cmdString, setup->pinNumber, valString);
	}

	delete gpioObj;

	return status;
}

// function to run pwm commands
int pwmRun(gpioSetup* setup)
{	//Gotta change this to either reference the Omega or Omega 2 object
	FastPwm		pwmObj;

	// check for correct command
	if (setup->cmd != GPIO_CMD_PWM) {
		return EXIT_FAILURE;
	}

	// object setup
	pwmObj.SetVerbosity(setup->verbose == FASTGPIO_VERBOSITY_ALL ? 1 : 0);
	pwmObj.SetDebugMode(setup->debug);


	// object operations
	pwmObj.Pwm(setup->pinNumber, setup->pwmFreq, setup->pwmDuty);

	return EXIT_SUCCESS;
}

// function to note the PID of the child process
int noteChildPid(int pinNum, int pid)
{
	char 	pathname[255];
	std::ofstream myfile;

	// determine thef file name and open the file
	snprintf(pathname, sizeof(pathname), PID_FILE, pinNum);
	myfile.open (pathname);

	// write the pid to the file
	myfile << pid;
	myfile << "\n";

	myfile.close();


	return EXIT_SUCCESS;
}

// function to read any existing pid notes and kill the child processes
int killOldProcess(int pinNum)
{
	char 	pathname[255];
	char	line[255];
	char	cmd[255];

	int 	pid;
	std::ifstream myfile;

	// determine thef file name and open the file
	snprintf(pathname, sizeof(pathname), PID_FILE, pinNum);
	myfile.open (pathname);

	// read the file
	if ( myfile.is_open() ) {
		// file exists, check for pid
		myfile.getline(line, 255);
		pid = atoi(line);

		// kill the process
		if (pid > 0)
		{
			sprintf(cmd, "kill %d >& /dev/null", pid);
			system(cmd);
			if (FASTGPIO_VERBOSE > 0) printf("Exiting current pwm process (pid: %d)\n", pid);
		}
	}


	return EXIT_SUCCESS;
}

// function to kill any old processes, based on which command is being run
int checkOldProcess(gpioSetup *setup)
{
	switch (setup->cmd) {
		case GPIO_CMD_SET:
		case GPIO_CMD_SET_DIRECTION:
		case GPIO_CMD_PWM:
			// kill the old process
			killOldProcess(setup->pinNumber);
			break;

		default:
			// do nothing
			break;
	}

	return EXIT_SUCCESS;
}


void pulse(FastGpio *gpioObj,int pinNum,int highMicros, int lowMicros)
{
	gpioObj->Set(pinNum,1);
	usleep(highMicros);
	gpioObj->Set(pinNum,0);
	usleep(lowMicros);
}


int pulseGpio(FastGpio *gpioObj,int pinNum, char* pathToFile, int repeats)
{
	const int MAX_CODE_SIZE = 200;
	gpioObj->SetDirection(pinNum,1);

	FILE * pFile;
	pFile = fopen (pathToFile,"r");
	int upTimes[MAX_CODE_SIZE];
	int downTimes[MAX_CODE_SIZE];
	int* pUpTimes = upTimes;
	int* pDownTimes = downTimes;

	// Load data from the file
	if (pFile != NULL)
	{
		int i = 0;

		while ((fscanf(pFile, "%d,%d\n", pUpTimes,pDownTimes) != EOF) && (i++ < MAX_CODE_SIZE))
		{
			pUpTimes++;
			pDownTimes++;
		}
		fclose (pFile);
	}
	else
	{
		printf("Error opening the file\n");
		return 1;
	}

	// Play the code
	while (repeats-- > 0)
	{
		pUpTimes = upTimes;
		pDownTimes = downTimes;
		while (*pUpTimes != 0)
		{
			// printf("Pulsing Up Time: %d, Down Time: %d\n",*pUpTimes,*pDownTimes);
			pulse(gpioObj,pinNum,*pUpTimes,*pDownTimes);

			pUpTimes++;
			pDownTimes++;
		}
	}
}

int main(int argc, char* argv[])
{
	int 		status;
	int 		ch;

	const char 	*progname;
	char		val[255];

	gpioSetup 	_setup;
	gpioSetup* 	setup = &_setup;

	// reset gpio setup and set defaults
	initGpioSetup(setup);

	setup->verbose 		= FASTGPIO_DEFAULT_VERBOSITY;
	setup->debug 		= FASTGPIO_DEFAULT_DEBUG;

	// save the program name
	progname = argv[0];


	//// parse the option arguments
	while ((ch = getopt(argc, argv, "vqud")) != -1) {
		switch (ch) {
		case 'v':
			// verbose output
			setup->verbose = FASTGPIO_VERBOSITY_ALL;
			break;
		case 'q':
			// quiet output
			setup->verbose = FASTGPIO_VERBOSITY_QUIET;
			break;
		case 'u':
			// ubus output
			setup->verbose = FASTGPIO_VERBOSITY_JSON;
			break;
		case 'd':
			// debug mode
			setup->debug 	= 1;
			break;
		default:
			usage(progname);
			return 0;
		}
	}

	frekvensRun(setup);
	return 0;

	// advance past the option arguments
	argc 	-= optind;
	argv	+= optind;

	// parse the arguments
	if (parseArguments(progname, argc, argv, setup) == EXIT_FAILURE) {
		usage(progname);
		return EXIT_FAILURE;
	}

	// check for any pwm processes already running on this pin
	status = checkOldProcess(setup);

	// run the command
	if (setup->cmd != GPIO_CMD_PWM) {
		// single gpio command
		status = gpioRun(setup);
	}
	else {
		//// continuous gpio commands, need another process

		// create the new process
		pid_t pid = fork();

		if (pid == 0) {
			// child process, run the pwm
			status = pwmRun(setup);
		}
		else {
			// parent process
			if (FASTGPIO_VERBOSE > 0) printf("Launched child pwm process, pid: %d \n", pid);
			noteChildPid(setup->pinNumber, pid);

			if ( setup->verbose == FASTGPIO_VERBOSITY_JSON ) {
				snprintf(val, sizeof(val), "%dHz with %d%% duty", setup->pwmFreq, setup->pwmDuty);
				printf(FASTGPIO_JSON_STRING, setup->cmdString, setup->pinNumber, val);
			}
		}
	}

	return 0;
}
