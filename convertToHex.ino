int convertToHex(int deci){
	char hexValue[10]; //init | increase value if need to store larger nums
  	sprintf(hexValue, "%X", deci); //convert to hex
  	int result = strtol(hexValue, NULL, 16); //convert to int value
  	return result;
}