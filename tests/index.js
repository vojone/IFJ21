const chalk = require("chalk");
const boxen = require("boxen");
const fs = require("fs")
const yargs = require("yargs");


const { exec } = require("child_process");

const greeting = chalk.white.bold("IFJCode21 TESTS!");
const boxenOptions = {
 padding: 1,
 margin: 1,
 borderStyle: "round",
 borderColor: "green"
};
var isWin = process.platform === "win32";

const msgBox = boxen( greeting, boxenOptions );

console.log(msgBox);

const options = yargs
 .usage("Usage: -n <name>")
 .option("n", { alias: "name", describe: "Your name", type: "string", demandOption: false })
 .argv;


var files = fs.readdirSync("./src")

console.log(files.filter(n => n.includes(".tl")))

files.forEach(filename => {

    exec("ls -la", (error, stdout, stderr) => {
        if (error) {
            console.log(`error: ${error.message}`);
            return;
        }
        if (stderr) {
            console.log(`stderr: ${stderr}`);
            return;
        }
        console.log(`stdout: ${stdout}`);
    });
    
})
