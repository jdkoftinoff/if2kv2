#include "jdk_world.h"
#include "if2k_kernel.h"
#include "if2k_config.h"
#include "if2k_settings.h"
#include "if2k_filter.h"

#if USE_PYTHON
#if JDK_IS_MACOSX
#include "Python/Python.h"
#else
#include "python2.3/Python.h"
#endif

void jdk_python_init()
{
  Py_Initialize();
}

void jdk_python_end()
{
  Py_Finalize();
}
#endif

int main( int argc, char **argv )
{
#if USE_PYTHON
  jdk_python_init();


//  PyObject *loc = PyDict_New ();
//  PyObject *glb = PyDict_New (); 
//  PyDict_SetItemString (glb, "__builtins__", PyEval_GetBuiltins ());
#endif  
  jdk_set_app_name( "if2003_test_kernel" );
  jdk_set_home_dir( "." );
  
#if USE_PYTHON
//  PyObject *code = Py_CompileString("print 'hello'", "<initialization>", Py_file_input );
//  PyEval_CallFunction(code,"()");
//  Py_XDECREF(code);  
//  if (PyErr_Occurred ())
//    PyErr_Print ();
#endif                   
  jdk_settings_text settings( 0, argc, argv );
  
  jdk_log_setup( 
    settings.get_long( "log_type" ),
    settings.get( "log_file" ),
    settings.get_long( "log_detail" )
    );				  
  
  
  if2_kernel_standard ifkernel( settings );
  ifkernel.update();
  
  
  // string to load text lines in
  jdk_str<4096> line_buffer;
  
  // read text lines until eof
  jdk_log( JDK_LOG_INFO, "Ready for input" );
  while( jdk_read_string_line( &line_buffer, stdin ) )
  {
    // search for patterns
    jdk_str<4096> matched_phrase;
    
    if2_kernel_result r=ifkernel.verify_url( "127.0.0.1", line_buffer.c_str(), matched_phrase, false, false );
    
    jdk_log( JDK_LOG_INFO, "result: %d %d '%s'\n", r.key, r.value, matched_phrase.c_str() );
  }
  
#if USE_PYTHON
  jdk_python_end();
#endif
  return 0;
}
