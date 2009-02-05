#!/usr/bin/perl -w

if (scalar(@ARGV) < 11) {
    print STDERR <<"EOF";
Usage:
  $0 rawconvert cycle0 cycleN  zx zy zz  dx dy dz  outroot file1 [file2 ...]

Examples:
  $0  ./rawconvert
                     0 3
                     120 120 720
                     2 2 2
                     out.%04d
                     ../p.%04d ../q.%04d


    Will take the eight raw 120x120x720 input files from the parent directory:
        p.0000 q.0000
        p.0001 q.0001
        p.0002 q.0002
        p.0003 q.0003

    ... and create the four time steps of silo files in the current directory:
        out.0000.root    out.0000.000.silo .. out.0000.007.silo
        out.0001.root    out.0001.000.silo .. out.0001.007.silo
        out.0002.root    out.0002.000.silo .. out.0002.007.silo
        out.0003.root    out.0003.000.silo .. out.0003.007.silo

    ... each of which contains the mesh "mesh" and the variables "p" and "q"
        and is decomposed into 8 domains.

EOF
    exit(0);
};

$prog     = shift;
$mincycle = shift;
$maxcycle = shift;
$x = shift;
$y = shift;
$z = shift;
$dx = shift;
$dy = shift;
$dz = shift;
$outroot  = shift;

print "creating $outroot, a ". ($maxcycle-$mincycle+1) ." frame animation\n";
print "    of the ${x}x${y}x${z} variables,\n";
print "    decomposing into ${dx}x${dy}x${dz} domains,\n";
print "    using program $prog\n";

@vars = @ARGV;

for ($i=$mincycle; $i<=$maxcycle; $i++) {
    $curroot = sprintf($outroot, $i);
    $cmdline = "$prog  $x $y $z   $dx $dy $dz   $curroot";
    foreach(@vars) {
        $curvar = sprintf($_, $i);
        $cmdline .= "  $curvar";
    }
    print "executing: $cmdline\n";
    `$cmdline`;
}
