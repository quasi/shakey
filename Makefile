CC = gcc
FLAGS = -c -g

srv : st.o support.o
        cc -o srv st.o support.o -lpthread

st.o: st.c
         ${CC} ${FLAGS} st.c

support.o: support.c
        ${CC} ${FLAGS} support.c

clean:
        rm -f *.o core srv
