//$Id: ring_wrap.cc,v 1.4 2005-10-05 06:46:59 bricken Exp $

#include <boost/python.hpp>
#include "mod2.h"
#include "ring_wrap.h"
#include "poly_wrap.h"
#include "febase.h"
static boost::python::object Ring_as_str(const Ring& r)
{
  using boost::python::str;
  StringSetS("");
  rWrite(r.pimpl);
  char* out=StringAppendS("");
  return boost::python::str(out,strlen(out));
}
void export_ring(){
boost::python::class_<Ring>("ring")
     .def("__str__", Ring_as_str)
     .def("set", &Ring::set)
     .def(boost::python::init <>());

}