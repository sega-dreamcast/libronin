#!/usr/local/bin/pike

constant dreamcast = "flute.bortas.org";
string cuafn = "0";

Stdio.File get_cua()
{
  string os = uname()->sysname;
  array sttycmd = ({"stty","57600","-parenb","cs8","-cstopb","crtscts",
		    "-clocal","ignbrk","ignpar","-parmrk","-inpck",
		    "-istrip","-inlcr","-icrnl","-igncr","-ixon","-ixoff",
		    "-opost","-icanon","-echo","-iexten", "min","1",
		    "time","0"});

  switch(lower_case(os)) {
   case "linux":
     cuafn = "/dev/ttyS"+ cuafn;
     break;
   case "sunos":
     sttycmd += ({ "crtsxoff", "-pendin" });
     cuafn = "/dev/cua"+ cuafn;
     break;
   default:
     werror("Serial fatal: Unknown OS '"+ os +"'. Unable to aquire serial!\n");
     return 0;
  }  

  Stdio.File cua = Stdio.File();
  if(!(cua->open(cuafn, "rw"))) {
    werror(cuafn+": "+strerror(cua->errno())+"\n");
    exit(1);
  }
  Process.create_process(sttycmd, (["stdin":cua]))->wait();
  
  return cua;
}

int main(int argc, array(string) argv)
{
  //Send ELF
  Stdio.File f = Stdio.File();
  write("Connecting... ");
  if( f->connect( dreamcast, 4711 ) )
  {
    write("Done\nWriting .. ");
    f->write( Stdio.stdin.read() );
  }
  f->close();
  write("Done\n");

  //Print out serial output.
  if( Stdio.File cua = get_cua() )
  {
    write("Listening to serial device...\n" );
    while(string q = cua->read(1024,1))
      write( q );
  }
}
