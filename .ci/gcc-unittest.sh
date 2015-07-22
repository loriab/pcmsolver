. "$CI_SCRIPTS/common.sh"

pip install cpp-coveralls --user `whoami`

python setup --cxx=g++ --cc=gcc --fc=gfortran --type=debug --tests --coverage
cd build
make
make test

coveralls --root .. -E ".*external.*" -E ".*CMakeFiles.*" -E ".*generated.*" -E ".*tests*" -E ".*tools.*" -E ".*cmake*" -E ".*doc*" -E ".*examples*" || echo 'coveralls upload failed.'
