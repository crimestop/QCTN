FC=clang++
#FC=g++-10

ALL := qctn_tools.o\
qctn.o\
qct.o

TEST := qctn_tools.o\
qctn.o\
qct.o \
.test.o

NETPATH := /Users/chaowang/Desktop/work/program/net
JSONPATH := /Users/chaowang/Desktop/work/program/json
GRAPHVIZPATH := /usr/local/Cellar/graphviz/2.44.1
TATPATH := /Users/chaowang/Desktop/work/program/TAT
TEBDPATH := /Users/chaowang/Desktop/work/program/TEBD
BOOSTPATH:= /Users/chaowang/Desktop/work/program/boost_1_75_0
MKLPATH:= /opt/intel/oneapi/mkl/2021.1.1
KAHYPAEPATH:= /Users/chaowang/Desktop/work/program/kahypar
USE_LAPACK := -lblas -llapack
USE_MKL :=  $(MKLPATH)/lib/libmkl_intel_lp64.a $(MKLPATH)/lib/libmkl_sequential.a $(MKLPATH)/lib/libmkl_core.a -lpthread -lm -ldl
LINKOPT := $(USE_MKL) -L /usr/local/lib -Wl,-rpath /usr/local/lib -lkahypar -L $(BOOSTPATH)/stage/lib -Wl,-rpath $(BOOSTPATH)/stage/lib -lboost_container -lboost_program_options  -L $(GRAPHVIZPATH)/lib -lgvc -lcgraph -Wl,-m
MAKEOPT := -g -std=c++17 -Ofast -march=native -I $(TATPATH)/include -DTAT_USE_EASY_CONVERSION -DTAT_USE_VALID_DEFAULT_TENSOR -DTAT_USE_SIMPLE_NAME -DTAT_USE_SINGULAR_MATRIX -DTAT_USE_SIMPLE_NOSYMMETRY -DTAT_USE_COPY_WITHOUT_WARNING -DNDEBUG -DTAT_USE_BOOST_PMR -I$(JSONPATH)/include -I$(TEBDPATH)/include  -I$(BOOSTPATH)  -I/usr/local/include  -I$(MKLPATH)/include -I $(GRAPHVIZPATH)/include -I$(NETPATH)/include -DNET_GRAPH_VIZ -DNET_SHOW_FIG_USE_ITERM -DNET_USE_LIB_KAHYPAR  -I $(KAHYPAEPATH)/ -DKAHYPAR_ENABLE_HEAVY_DATA_STRUCTURE_ASSERTIONS -DKAHYPAR_ENABLE_HEAVY_PREPROCESSING_ASSERTIONS -DKAHYPAR_ENABLE_HEAVY_COARSENING_ASSERTIONS -DKAHYPAR_ENABLE_HEAVY_INITIAL_PARTITIONING_ASSERTIONS -DKAHYPAR_ENABLE_HEAVY_REFINEMENT_ASSERTIONS

all: test.out

test.out:$(TEST)
	$(FC) $(FFLAGS) -o $@ $^ $(LINKOPT)
.%.o :	%.cpp
	$(FC) $(FFLAGS) -o $@ -c $(<) $(MAKEOPT)
%.o :	source/%.cpp
	$(FC) $(FFLAGS) -o $@ -c $(<) $(MAKEOPT)
clean:
	rm .*.o *.o *.mod *.out 
.PHONY:	all clean test.out
