const chalk = require("chalk");
const boxen = require("boxen");

// const greeting = chalk.white.bold("IFJCode21 TESTS!");

const boxenOptions = {
 padding: 1,
 margin: 1,
 borderStyle: "round",
 borderColor: "green"
};
const msgBox = boxen( greeting, boxenOptions );


const yargs = require("yargs");

const options = yargs
 .usage("Usage: -n <name>")
 .option("n", { alias: "name", describe: "Your name", type: "string", demandOption: true })
 .argv;

const greeting = `Hello, ${options.name}!`;

console.log(greeting);

console.log(msgBox);

var isWin = process.platform === "win32";

