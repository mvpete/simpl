### Object Examples ###

object vehicle
{
    color;
    wheel_count;
}

object car inherits vehicle
{
}

object truck inherits vehicle
{
}

object terrain
{
}

object pavement inherits terrain
{
}

object mud inherits terrain
{
}

def drive(v is vehicle, t is terrain)
{
    print("Vehicle on terrain");
}

def drive(v is truck, t is pavement)
{
    print("Truck on pavement");
}

def drive(c is car, p is pavement)
{
    print("Car on pavement");
}

def drive(t is truck, m is mud)
{
    print("Truck on mud");
}


let v = new vehicle {};
let t = new terrain {};

drive(v,t);