"""Generate a scalable IGNITE faceplate, using the supplied flat render's layout."""
from html import escape
from math import cos, sin, radians
from pathlib import Path

parts = ['''<svg xmlns="http://www.w3.org/2000/svg" width="1496" height="956" viewBox="0 0 748 478">
<defs>
 <linearGradient id="body" x2="0" y2="1"><stop stop-color="#303237"/><stop offset="1" stop-color="#1d1f23"/></linearGradient>
 <linearGradient id="key" x2="0" y2="1"><stop stop-color="#373a40"/><stop offset=".18" stop-color="#24262b"/><stop offset="1" stop-color="#14161a"/></linearGradient>
 <radialGradient id="knob"><stop stop-color="#343840"/><stop offset=".55" stop-color="#111317"/><stop offset=".83" stop-color="#30343b"/><stop offset="1" stop-color="#08090b"/></radialGradient>
 <linearGradient id="metal" x2="1" y2="1"><stop stop-color="#747b87"/><stop offset=".3" stop-color="#20242b"/><stop offset=".65" stop-color="#525963"/><stop offset="1" stop-color="#171a20"/></linearGradient>
</defs>
<rect x="1" y="1" width="746" height="476" rx="7" fill="url(#body)" stroke="#4a4d54" stroke-width="2"/>
<rect x="4" y="4" width="740" height="470" rx="5" fill="none" stroke="#101115"/>
<rect x="72" y="17" width="316" height="197" rx="2" fill="#08090c" stroke="#3a3c42"/>
<rect x="88" y="33" width="284" height="165" fill="#080c15" stroke="#161b25"/>
<rect x="416" y="19" width="51" height="23" rx="2" fill="#2b2e33" stroke="#383b40"/>
<path d="M421 36 L440 24 L460 36 Z" fill="#3b3f45"/>
''']


def text(x, y, value, size=6.4, color='#d4d6da', weight='bold'):
    parts.append(f'<text x="{x}" y="{y}" text-anchor="middle" font-family="Arial, sans-serif" font-size="{size}" font-weight="{weight}" fill="{color}">{escape(value)}</text>')


def key(x, y, label, w=35, h=20, color='#b7bcc6'):
    parts.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="2" fill="url(#key)" stroke="#0c0d10" stroke-width="1.2"/>')
    parts.append(f'<path d="M{x+3} {y+2} H{x+w-3}" stroke="#52565e" stroke-width=".6"/>')
    for i, line in enumerate(label.split('|')):
        text(x+w/2, y+h/2+2+(i-(len(label.split('|'))-1)/2)*6.5, line, 5.6, color)


def knob(x, y, radius=17, ticks=False):
    parts.append(f'<circle cx="{x}" cy="{y}" r="{radius+1}" fill="#0b0c0f" stroke="#454951" stroke-width=".7"/>')
    if ticks:
        for i in range(13):
            angle = radians(-220 + i * 21.7)
            a, b = radius+3.5, radius+6
            parts.append(f'<path d="M{x+cos(angle)*a:.2f} {y+sin(angle)*a:.2f} L{x+cos(angle)*b:.2f} {y+sin(angle)*b:.2f}" stroke="{ "#f0657e" if i < 7 else "#51545e" }" stroke-width="1.6"/>')
    else:
        parts.append(f'<circle cx="{x}" cy="{y}" r="{radius-1}" fill="none" stroke="#d75d74" stroke-width="1.3"/>')
    parts.append(f'<circle cx="{x}" cy="{y}" r="{radius-3}" fill="url(#knob)"/>')
    parts.append(f'<circle cx="{x}" cy="{y}" r="{radius-6}" fill="url(#metal)" stroke="#15171c" stroke-width=".6"/>')
    parts.append(f'<path d="M{x-1} {y-radius+7} L{x+1} {y-radius+7} L{x+.5} {y-3} L{x-.5} {y-3} Z" fill="#d9dee5"/>')


text(230, 28, 'behringer', 10, '#d2d4d8', 'normal')
text(230, 208, 'IGNITE', 12, '#edeef1')
text(661, 28, '64-CHANNEL MULTI-TRACK SEQUENCER', 5.5, '#9299a5')
key(25, 44, 'USB', 31, 41)
parts.append('<rect x="32" y="74" width="17" height="3" rx="1" fill="#dc6859"/>')
key(27, 116, 'USB|STOP', 28, 21)
key(27, 167, 'MENU', 28, 19, '#df7386')
text(47, 245, 'ROW', 7)
knob(47, 270, 20)
text(700, 245, 'BAR', 7)
knob(700, 270, 20)
parts.append('<rect x="401" y="86" width="82" height="83" rx="11" fill="url(#key)" stroke="#08090c"/>')
key(404, 90, 'BACK', 20, 19)
key(461, 90, 'COPY', 20, 19)
key(404, 147, '−', 20, 18)
key(461, 147, '+', 20, 18)
knob(442, 126, 27)
text(442, 181, 'JOG / ENTER', 5.7, '#838b99')
names = ['RELEASE', 'CUTOFF', 'RESONANCE', 'VOLUME', 'REVERB', 'CHORUS', 'EQ HIGH', 'EQ LOW']
for i, label in enumerate(names):
    x, y = 523+(i%4)*60, 88 if i < 4 else 166
    text(x, y-27, label, 6.1)
    knob(x, y, 14, ticks=True)
    text(x, y+25, chr(65+i), 6.2, '#8e97a5')

columns = [96, 152, 209, 271, 328, 385, 442, 503, 560, 617]
upper = ['SHIFT', 'LAST', 'SAVE', '●', '◀◀', '■', '▶', 'SCULPT', 'DELETE', 'INSERT']
lower = ['SONG', 'SCENE', 'TRACK', 'NEXT/TIE', 'FILL/X', 'SOLO/SKIP', 'GANG', 'PATTERN', 'TEMPO', 'INTERNAL|SOUNDS']
for i, x in enumerate(columns):
    transport = 3 <= i <= 6
    if transport:
        key(x, 238, '', 35, 35)
        color = ['#e46e65', '#80afdb', '#efc567', '#87c478'][i-3]
        if i == 3:
            parts.append(f'<circle cx="{x+17.5}" cy="255.5" r="4" fill="{color}"/>')
        elif i == 5:
            parts.append(f'<rect x="{x+14}" y="252" width="7" height="7" fill="{color}"/>')
        elif i == 6:
            parts.append(f'<path d="M{x+14} 251 L{x+22} 255.5 L{x+14} 260 Z" fill="{color}"/>')
        else:
            parts.append(f'<path d="M{x+19} 251 V260 L{x+13} 255.5 Z M{x+25} 251 V260 L{x+19} 255.5 Z" fill="{color}"/>')
    else:
        key(x, 240, upper[i])
    key(x, 285, lower[i])
parts.append('<path d="M98 264 H129" stroke="#b6c6ab" stroke-width="1.2"/>')
colors = ['#ce604d', '#b95ca1', '#76b9d0', '#b5c77b']
for i in range(16):
    x = 34 + int(i*45.2)
    knob(x, 356, 16)
    if i % 4 == 0:
        parts.append(f'<rect x="{x-12}" y="382" width="24" height="7" fill="none" stroke="#878d98" stroke-width=".5"/>')
    text(x, 388, str(i+1), 6.5, '#c4cad2')
    parts.append(f'<rect x="{x-13}" y="398" width="26" height="43" rx="1.7" fill="#111318" stroke="#08090b"/>')
    parts.append(f'<rect x="{x-11}" y="400" width="22" height="39" rx="1" fill="{colors[i%4]}"/>')
    parts.append(f'<path d="M{x-9} 402 H{x+9}" stroke="#ffffff" stroke-opacity=".42" stroke-width="1"/>')
    parts.append(f'<path d="M{x-10} 436 H{x+10}" stroke="#000000" stroke-opacity=".3" stroke-width="2"/>')
text(374, 462, 'DESKTOP SIMULATOR', 6.5, '#667080', 'normal')
parts.append('</svg>')
target = Path(__file__).resolve().parents[1] / 'Source/Application/assets/ignite-panel.svg'
target.write_text('\n'.join(parts), encoding='utf-8')
print(target)
