# Possible errors #

## send_file ##

* missing parameters  
  --> error/abort

* file_info fails
  * file not found  
  --> skip
  * file not an ordinary file (i.e., socket, symlink, directory)
  --> skip

* link() fails
  * source file not found  
    --> abort
  * permission denied - source  
    --> abort
  * permission denied - target  
    --> abort
  * xdev link  
    --> fall back to copy

* write meta file fails
  * permission denied
  * filesystem full
  * file exists

* unlink() fails
  * permission denied
  * file not found

## retrieve_file ##

?

## query_available ##

?

## collector info ##

?
