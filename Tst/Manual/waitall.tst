LIB "tst.lib"; tst_init();
  link l1 = "ssi:fork"; open(l1);
  link l2 = "ssi:fork"; open(l2);
  link l3 = "ssi:fork"; open(l3);
  list l = list(l1,l2,l3);
  write(l1, quote(system("sh", "sleep 15")));
  write(l2, quote(system("sh", "sleep 10")));
  write(l3, quote(system("sh", "sleep 11")));
  waitall(l, 5000); // terminates after 5sec with result 0
  waitall(l);       // terminates after 10 more sec
  close(l1);
  close(l2);
  close(l3);
tst_status(1);$
