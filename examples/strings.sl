# Strings example
@import io
@import string

let hello = "hello";
let world = "world";

println(length(hello));
println(concat(concat(hello, " "), world));

for(let i=0; i<length(hello); ++i) {
	println(at(hello, i));
}

println(substr(hello, 0, 4));

let list = "oranges,apples,bananas";
let arr = split(list, ",");

println(arr);
println(join(arr, ";"));

println(format("({0},{1})", new [hello, world]));
