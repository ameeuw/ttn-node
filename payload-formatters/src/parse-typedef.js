const fs = require("fs");

function cleanLine(line) {
  return line.trim().replace(/\n/g, "");
}

function parseStructsFromFile(filePath, structIdentifier = "fport") {
  const fileContent = fs.readFileSync(filePath, "utf8");

  /* Matches a struct definition in the file content using a
   * regex pattern that matches the struct keyword followed
   * by a struct name and the struct body between curly braces
   * and terminated by a semicolon ; at the end of the struct definition block
   */
  const structRegex = /\bstruct\s+\w+\s*{[^}]*};/g;

  const matches = fileContent.match(structRegex);
  /* Extracts the struct id and name from the struct definition
   * using regex patterns that match the struct identifier
   * and the struct name respectively
   */
  const structs = matches.map((structString) => ({
    /* Extracts the struct id from the struct definition
     * using a regex pattern that matches the struct identifier
     * and captures the id as a group
     */
    id: Number(
      structString.match(new RegExp(`${structIdentifier} = (\\d+);`))[1]
    ),
    /* Extracts the struct name from the struct definition
     * using a regex pattern that matches the struct keyword
     * followed by a struct name and captures the name as a group
     */
    name: structString.match(/struct\s+(\w+)/)[1],
    /* Removes newlines from the struct definition and
     * adds it to the struct object
     */
    structString: structString.split("\n").map(cleanLine).join(""),
  }));
  return structs;
}

module.exports = {
  parseStructsFromFile,
};
