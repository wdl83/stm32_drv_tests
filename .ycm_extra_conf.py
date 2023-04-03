import platform
import os.path as p
import subprocess

DIR_OF_THIS_SCRIPT = p.abspath( p.dirname( __file__ ) )
SOURCE_EXTENSIONS = [ '.c' ]

flags = [
'-DAPB1_CLK=36000000',
'-DAPB2_CLK=72000000',
'-DCPU_CLK=72000000',
'-DSTM32F103C8',
'-I.',
'-Istm32_drv',
'-Wall',
'-Werror',
'-Wextra',
'-Wno-parentheses',
'-Wno-unused-parameter',
'-Wunreachable-code'
'-march=armv7-m',
'-mcpu=cortex-m3',
'-mthumb'
'-std=gnu99',
]

def IsHeaderFile( filename ):
  extension = p.splitext( filename )[ 1 ]
  return extension in [ '.h' ]


def FindCorrespondingSourceFile( filename ):
  if IsHeaderFile( filename ):
    basename = p.splitext( filename )[ 0 ]
    for extension in SOURCE_EXTENSIONS:
      replacement_file = basename + extension
      if p.exists( replacement_file ):
        return replacement_file
  return filename

def Settings( **kwargs ):
  # Do NOT import ycm_core at module scope.
  import ycm_core

  language = kwargs[ 'language' ]

  if language == 'cfamily':
    filename = FindCorrespondingSourceFile( kwargs[ 'filename' ] )

    return {
      'flags': flags,
      'include_paths_relative_to_dir': DIR_OF_THIS_SCRIPT,
      'override_filename': filename
    }

  return {}
