### Object Examples ###
@import io

object vehicle
{
    color;
    wheel_count;
}

object car inherits vehicle
{
    color="Blue";
}

object truck inherits vehicle
{
    color=10; ## this is okay
    ## color; ## this is not. redefinition of member.
}

object terrain
{
    difficulty;
}

object pavement inherits terrain
{
}

object mud inherits terrain
{
}

def drive(v is vehicle, t is terrain)
{
    print(v.color);
    println(" Vehicle on terrain");
}

def drive(v is truck, t is pavement)
{
    print(v.color);
    println(" Truck on pavement");
}

def drive(c is car, p is pavement)
{
    print(c.color);
    println(" Car on pavement");
}

def drive(t is truck, m is mud)
{
    println("Truck on mud");
}

let vh = new vehicle {};
let tr = new terrain {};

drive(vh,tr);

vh = new car {};

drive(vh,tr);

tr = new pavement {};

drive(vh,tr);

vh = new vehicle {};

drive(vh, tr);

vh = new truck {};

drive(vh, tr);

