####  Blob Example ####
#   Blobs are a special data type in simpl
#   they are typeless objects. Or, blobs. 
#   They function just like a map. That's
#   because, well, they are.
####               ####

def deal() {
    print(take(deck,5));
}

let faces = new ["1","2","3","4","5","6","7","8","9","10","J","Q","K"];
let suits = new ["D","H","S","C"];

let deck = new [];
for(let i=0; i<size(suits); i=i+1)
{
   for(let j=0; j<size(faces); j=j+1)
   {
       let card = new { face=faces[j], suit=suits[i] };
       push(deck, card);
   }
}

shuffle(deck);

deal();

