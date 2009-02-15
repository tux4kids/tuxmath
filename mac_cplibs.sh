#! /bin/sh

# A function that takes as input a binary name and a lib target directory
# cpswlibs <binaryname> <directory to copy dependencies to>
function cpswlibs {
# Get the list of dependencies in /sw, excluding self
fls=$(otool -L $1 | grep "/sw" | cut -f 2 | cut -d "(" -f 1 | grep -v $1)
echo $fls
# Copy these dependencies into the target directory
cp $fls $2
}

# Run cpswlibs on the input binary name
BINNAME=$1
LIBDIRNAME=$2
cpswlibs $BINNAME $LIBDIRNAME
# Now run cpswlibs on each of the libraries that we copied
echo $LIBDIRNAME
fls=$(ls $LIBDIRNAME)
for i in $fls; do
    echo $i
    cpswlibs $LIBDIRNAME/$i $LIBDIRNAME
done
# For tuxmath we don't need any more recursions that this