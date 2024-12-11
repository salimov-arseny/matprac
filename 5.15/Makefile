CXX = g++

CXXFLAGS = -lpthread -O3 -mfpmath=sse -fstack-protector-all -g -W -Wall -Wextra -Wunused -Wcast-align -Werror -pedantic -pedantic-errors -Wfloat-equal -Wpointer-arith -Wformat-security -Wmissing-format-attribute -Wformat=1 -Wwrite-strings -Wcast-align -Wno-long-long -Woverloaded-virtual -Wnon-virtual-dtor -Wcast-qual -Wno-suggest-attribute=format

OBJS = main.o func.o

TARGET = a.out

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp header.h
	$(CXX) $(CXXFLAGS) -c main.cpp

func.o: func.cpp header.h
	$(CXX) $(CXXFLAGS) -c func.cpp

clean:
	rm -f $(OBJS) $(TARGET)
#del /Q $(OBJS) $(TARGET)
