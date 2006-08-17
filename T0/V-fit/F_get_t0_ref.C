float F_get_t0_ref(char *det)
{
  float t0_ref = -1;
  ifstream myfile ("coral-t0.txt");
  if (myfile.is_open())
  {
    char d[20],t0[20];
    while (myfile >> d >> t0)
    {
        if(!strcmp(d,det))
        {
            t0_ref = atof(t0);
            return t0_ref;

        }
    }
    myfile.close();
  }

  else cout << "Unable to open coral-t0.txt"; 

//if it failed it retruns the initials ridiculuous value of -1
return t0_ref;
}
