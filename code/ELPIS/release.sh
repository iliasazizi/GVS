rm -r Release ; mkdir Release
cd Release
cmake -DCMAKE_BUILD_TYPE=Release .. &> logCMAKE.log
make
echo "_Release_Mode Project! Pleasee find the exec in release repository."
