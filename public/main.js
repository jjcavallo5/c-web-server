console.log("Loaded");
const button = document.getElementById("btn");

button.addEventListener("click", async () => {
  const resp = await fetch("/endpoint");
  const body = await resp.text();

  if (!resp.ok) throw Error(body);

  document.getElementById("response-container").setHTMLUnsafe(`<p>${body}</p>`);
});
