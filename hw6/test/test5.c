int MAIN()
{
  int i, num1;
  int result = 0;
  
  write("enter a integer\n");
  num1 = read();
 
  for (i = 0; i < num1; i = i + 1)
    {
      result = result + i;
    } 
  write(result);  
    write("\n");
  return 0; 
}

