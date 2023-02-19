
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

println(split(list, ","));
