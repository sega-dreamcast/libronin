#!/usr/bin/env pike

mapping(int:string) bottled_answer = ([]);
array(int) bottle_queue = ({});

mapping(int:Stdio.File|array(string)) fds = ([]);

string cwd=".";

string low_fixfn(string fn)
{
  string tmp;
  while((tmp = replace(fn, "//", "/")) != fn) fn = tmp;
  if(fn[..0]=="/")
    return (fn == "/"? "." : fn[1..]);
  else
    return combine_path(cwd, fn);
}

string fixfn(string fn)
{
  array(string) fns = map(({fn, upper_case(fn), lower_case(fn)}), low_fixfn);
  foreach(fns, string ff)
    if(file_stat(ff))
      return ff;
  return fns[0];
}

string bottle_answer(int ser, int res, string|void extra)
{
  if(sizeof(bottle_queue)>=10) {
    m_delete(bottled_answer, bottle_queue[0]);
    bottle_queue = bottle_queue[1..];
  }
  if(res == -1) --res;
  bottle_queue += ({ ser });
  return bottled_answer[ser] = sprintf("%-4c%-4c%s", ser, res, extra||"");
}

int generate_fd()
{
  if(!sizeof(fds))
    return 3;
  return sort(indices(fds))[-1]+1;
}

string process_command(int ser, int cmd, string data)
{
  int fd, pos, len;
  Stdio.Stat st;
  if(bottled_answer[ser])
    return bottled_answer[ser];
  switch(cmd) {
  case 1:
    fd = generate_fd();
    Stdio.File f = Stdio.File();
    if(f->open(fixfn(data), "r")) {
      fds[fd] = f;
      return bottle_answer(ser, fd);
    } else
      return bottle_answer(ser, -2);
  case 2:
    if(sscanf(data, "%-4c%-4c%-4c", fd, pos, len)==3 &&
       fds[fd] && objectp(fds[fd]) && fds[fd]->seek(pos)>=0 &&
       (data = fds[fd]->read(len)))
      return bottle_answer(ser, sizeof(data), data); 
    else
      return bottle_answer(ser, -2);
  case 3:
    if(sscanf(data, "%-4c", fd)==1 &&
       fds[fd] && objectp(fds[fd]) && fds[fd]->close()) {
      m_delete(fds, fd);
      return bottle_answer(ser, 0);      
    } else
      return bottle_answer(ser, -2);
  case 4:
    fd = generate_fd();
    array dir;
    if(dir = get_dir(fixfn(data))) {
      fds[fd] = ({ fixfn(data) }) + dir; //Array.map(dir, upper_case);
      //werror("Dir: %O\n", Array.map(dir, upper_case));
      return bottle_answer(ser, fd);
    } else
      return bottle_answer(ser, -2);
  case 5:
    if(sscanf(data, "%-4c", fd)==1 &&
       fds[fd] && arrayp(fds[fd])) {
      m_delete(fds, fd);
      return bottle_answer(ser, 0);      
    } else
      return bottle_answer(ser, -2);
  case 6:
    if(sscanf(data, "%-4c%-4c", fd, pos)==2 &&
       fds[fd] && arrayp(fds[fd]) &&
       pos>=0 && pos+1<sizeof(fds[fd]) &&
       (st=file_stat(combine_path(fds[fd][0], fds[fd][pos+1])))) {
      return bottle_answer(ser, 0, sprintf("%-4c%s", st[1], fds[fd][pos+1]));
    } else
      return bottle_answer(ser, -2);
  case 7:
    string newcwd = fixfn(data);
    if(file_stat(newcwd)) {
      cwd = newcwd;
      return bottle_answer(ser, 0);      
    } else
      return bottle_answer(ser, -2);
  case 8:
    if(sscanf(data, "%-4c", fd)==1 &&
       fds[fd] && objectp(fds[fd]) && (st=fds[fd]->stat())) {
      return bottle_answer(ser, st[1]);
    } else
      return bottle_answer(ser, -2);
  }
}

int main()
{
  Stdio.File f;
  Stdio.UDP u = Stdio.UDP();
#if 0
  int pktcnt = 0;
  signal(signum("SIGQUIT"), lambda() { write("[pktcnt=%d]\n", pktcnt); });
  Stdio.UDP()->bind(1443)->set_read_callback(lambda() {
					       if(!pktcnt)
						 write("[Connect]\n");
					       ++pktcnt;
					     });
#endif
  Stdio.UDP()->bind(1445)->set_read_callback(lambda(mapping pkt) {
					       write(pkt->data);
					     });
  Stdio.UDP()->bind(1447)->set_read_callback(lambda(mapping pkt) {
					       if(!f) {
						 f = Stdio.File();
						 f->open("oob_data", "cwt");
					       }
					       f->write(pkt->data);
					     });
  u->bind(1451)->set_read_callback(lambda(mapping pkt) {
				     int ser, res, cmd;
				     string data;
				     if(4 == sscanf(pkt->data, "%-4c%-4c%-4c%s",
						    ser, res, cmd, data) &&
					res == 4294967295)
				       data = process_command(ser, cmd, data);
				     if(data)
				       u->send(pkt->ip, pkt->port, data);
				   });

  signal(signum("SIGQUIT"), lambda() { write("[Open files: %d]\n", sizeof(fds)); });
  
  return -17;
}
