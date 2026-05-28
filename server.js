const express = require('express');
const fs = require('fs');
const { exec } = require('child_process');
const path = require('path');

const app = express();
app.use(express.json());

// Serve static files from the root directory (to serve index.html)
app.use(express.static(__dirname));

// Determine the path to the compiled C++ executable
const BACKEND_DIR = path.join(__dirname, 'backend');
const COMPILER_EXEC = path.join(BACKEND_DIR, process.platform === 'win32' ? 'main.exe' : 'main');

app.post('/api/compile', (req, res) => {
    const { code } = req.body;
    
    if (!code) {
        return res.status(400).json({ error: "No source code provided." });
    }

    if (!fs.existsSync(COMPILER_EXEC)) {
        return res.status(500).json({ 
            error: `Compiler executable not found at:\n${COMPILER_EXEC}\n\nPlease compile your C++ backend first by running:\ncd backend && g++ *.cpp -o main.exe` 
        });
    }

    // Generate unique temp files to prevent race conditions if multiple people use it
    const uniqueId = Date.now();
    const inputPath = path.join(__dirname, `temp_input_${uniqueId}.txt`);
    const outputPath = path.join(__dirname, `temp_output_${uniqueId}.cpp`);

    // 1. Write the frontend's text input to a temp file
    fs.writeFileSync(inputPath, code);

    // 2. Execute the C++ compiler, passing the temp paths as arguments
    const command = `"${COMPILER_EXEC}" "${inputPath}" "${outputPath}"`;

    exec(command, (error, stdout, stderr) => {
        // Cleanup the temporary input file immediately
        if (fs.existsSync(inputPath)) fs.unlinkSync(inputPath);

        if (error) {
            return res.status(500).json({ 
                error: `Compiler execution failed.\n\n[STDERR]\n${stderr}\n\n[STDOUT]\n${stdout}` 
            });
        }

        // 3. Read the generated C++ output file
        if (fs.existsSync(outputPath)) {
            const compiledCode = fs.readFileSync(outputPath, 'utf8');
            fs.unlinkSync(outputPath); // Clean up output file
            res.json({ output: compiledCode, logs: stdout });
        } else {
            res.status(500).json({ error: "Compilation finished, but the generated output file was not found.\n\nLogs:\n" + stdout });
        }
    });
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}`);
    console.log(`Expecting C++ compiler at: ${COMPILER_EXEC}`);
});