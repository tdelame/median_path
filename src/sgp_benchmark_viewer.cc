/*  Created on: Mar 29, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */
# include "simple_qml_application.h"
# include "simple_camera.h"
# include "simple_gl_window.h"

# include <graphics-origin/tools/log.h>

# include <boost/program_options.hpp>
# include <boost/filesystem.hpp>

# include <sys/ioctl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <iostream>

# include <QGuiApplication>
# include <QApplication>

static const std::string version_string =
  "SGP 2016 Benchmark Viewer tool v1.0 ©2016 Thomas Delame";

static const std::string help_string =
  "SGP 2016 Benchmark Viewer\n"\
  "=========================\n\n"\
  "Viewer of the benchmark results.\n"
  ;

static uint32_t get_console_line_length()
{
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_col;
}

namespace po = boost::program_options;

struct application_parameters {
  std::string input_stem;
  std::string input_directory;
  std::string extension;

  application_parameters()
  {}


  application_parameters( int argc, char* argv[] )
  {
    auto line_length = get_console_line_length();
    po::options_description generic(
        "Generic options", line_length,
        line_length > 10 ? line_length - 10 : line_length );

    generic.add_options()
     ("version,v", "print version string")
     ("help,h", "produce help message")
     ("input_directory,d", po::value<std::string>(&input_directory),
         "where the benchmark results had been stored. If not specified, this is the working directory.")
     ("stem,s", po::value<std::string>(&input_stem),
         "stem of the input skeletons to compare (the basename of the input mesh used to compute the skeleton, from which we removed the extension part)")
     ("format,f", po::value<std::string>(&extension)->default_value("median"),
         "format of the skeleton files to load (median, web)")
    ;

    po::variables_map vm;
    po::store(
        po::command_line_parser(argc, argv ).options( generic ).run(),
        vm);
    po::notify(vm);

    if( vm.count("version") )
      std::cout << version_string << std::endl;

    if( vm.count("help") )
      {
        std::cout<< help_string << generic << std::endl;
        exit( EXIT_SUCCESS );
      }

    if( input_directory.empty() )
      {
        boost::filesystem::current_path().string();
      }

    if( input_stem.empty() )
      {
        LOG( error, "no input stem. Please specify one input stem to launch the viewer");
        std::cout << help_string << generic << std::endl;
        exit( EXIT_FAILURE );
      }
  }
};

int main( int argc, char* argv[] )
{
  application_parameters params = application_parameters( argc, argv );

  int dummy_argc = 1;
//  QGuiApplication qgui( dummy_argc, argv );
  QApplication qgui( dummy_argc, argv );

  qmlRegisterType<simple_gl_window>("MedianPath", 1, 0, "GLWindow");
  qmlRegisterType<median_path::simple_camera>("MedianPath", 1, 0, "GLCamera");

  median_path::simple_qml_application app;
  app.setSource( QUrl::fromLocalFile("qml/Viewer.qml"));
  app.show();

  auto window = app.rootObject()->findChild<simple_gl_window*>("glwindow");
  window->load_benchmark( params.input_stem, params.input_directory, params.extension );

  app.raise();
  return qgui.exec();
}
