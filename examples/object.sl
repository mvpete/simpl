### Object Examples ###

object vehicle
{
    color;
    wheel_count;
}

object car inherits vehicle
{
    color="blue";
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

def drive(v,t)
{
    println("Generic drive!");
}

def drive(v is vehicle, t is terrain)
{
    println("Vehicle on terrain");
}

def drive(v is truck, t is pavement)
{
    println("Truck on pavement");
}

def drive(c is car, p is pavement)
{
    println("Car on pavement");
}

def drive(t is truck, m is mud)
{
    println("Truck on mud");
}

drive("car", "dirt");

let vh = new vehicle {};
let tr = new terrain {};

drive(vh,tr);

vh = new car {};

drive(vh,tr);

tr = new pavement {};

drive(vh,tr);

vh = new vehicle {};

drive(vh, tr);
