const fs = require('fs');
const path = require('path');
const hwDir = path.join(process.cwd(), '..', 'HARDWARE');
console.log('CWD:', process.cwd());
console.log('Target HW Dir:', hwDir);
try {
  const files = fs.readdirSync(hwDir);
  console.log('Files in HARDWARE:', files);
} catch (e) {
  console.error('Error reading HARDWARE:', e.message);
}
