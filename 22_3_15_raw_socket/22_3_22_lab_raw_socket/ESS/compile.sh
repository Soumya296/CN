
  
for i in *.cpp
do
    g++ -g3 -o3 $i -o ${i%.cpp} -lpthread
done