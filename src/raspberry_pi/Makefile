all: single_finger

CXX = g++

EIGEN_INC_PATH = /home/cocoa_kang/LIBRARIES/eigen/
LIB_INC_PATH = /usr/local/include/
LIB_LINK_PATH = /usr/local/lib/

HDRS = I2Cdev.h MPU6050.h helper_3dmath.h dragon.h glove_state.h thread.h
SF_OBJS_K = main_k.o I2Cdev.o MPU6050.o dragon.o glove_state.o glove.o thread.o MPU6050_6Axis_MotionApps20.o

$(SF_OBJS) : $(HDRS)
MPU6050_6Axis_MotionApps20.o : MPU6050_6Axis_MotionApps20.cpp $(HDRS)
	g++ -c MPU6050_6Axis_MotionApps20.cpp -fopenmp -I $(LIB_INC_PATH)
dragon.o : dragon.cpp $(HDRS)
	g++ -c dragon.cpp -fopenmp -I $(LIB_INC_PATH)
glove_state.o : glove_state.cpp $(HDRS)
	g++ -c glove_state.cpp -fopenmp -I $(LIB_INC_PATH)
glove.o : glove.cpp $(HDRS)
	g++ -c glove.cpp -fopenmp -I $(LIB_INC_PATH)
thread.o : thread.cpp $(HDRS)
	g++ -c thread.cpp -fopenmp -I $(LIB_INC_PATH)
main_k.o : main_k.cpp $(HDRS)
	g++ -c main_k.cpp -fopenmp -I $(EIGEN_INC_PATH) -I $(LIB_INC_PATH)
I2Cdev.o : I2Cdev.cpp $(HDRS)
	g++ -c I2Cdev.cpp -fopenmp
MPU6050.o : MPU6050.cpp $(HDRS)
	g++ -c MPU6050.cpp -fopenmp

single_finger: $(SF_OBJS_K)
	$(CXX) -o $@ $^ -fopenmp -lm -L $(LIB_LINK_PATH) $(OPENGL_LINKS) -lbcm2835

clean:
	rm -f $(SF_OBJS) single_finger

