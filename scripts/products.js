//here we recover the element from HTML file 
const prev = document.getElementById("prev-btn");
const next = document.getElementById("next-btn");
const list = document.getElementById("item-list");

const itemWidth = 150;
const padding = 10;

//add the event listener on the click on the prev button
prev.addEventListener("click", () => {
  list.scrollLeft -= itemWidth + padding;
});

//add the event listener on the click on the next button
next.addEventListener("click", () => {
  list.scrollLeft += itemWidth + padding;
});