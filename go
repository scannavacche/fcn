#!/bin/bash
clear

compile_run() {
  ./comp++ $1
  RESULT=$?
  if [ $RESULT -eq 0 ] || [ $RESULT == 0 ]; then
      echo success
      ./$1
  else
    echo failed
  fi
}

for arg in "$@"
do
    case $arg in
        "sample")
          compile_run "es1_campionamento" 
        ;;
        "exp")
          compile_run "es2_taylor_e"
        ;;
        "cos")
          compile_run "es2_taylor_cos" 
        ;;
        "prec")
          compile_run "es3_precisione" 
        ;;
        "deriv")
          compile_run "es1_derivate"
        ;;
        "cauchy")
          compile_run "es1_cauchy" 
        ;;
        "bordo")
          compile_run "es1_bordo" 
        ;;
        "all")
          compile_run "fcn_all" 
        ;;
   esac
done
