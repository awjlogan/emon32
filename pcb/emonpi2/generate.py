#!/usr/bin/env python3

import os
import platform
import re
import shlex
import subprocess


def gitrev():
    git_cmd = "git rev-parse --short HEAD"
    try:
        rev = subprocess.run(shlex.split(git_cmd),
            capture_output=True,
            check=True,
            text=True
            ).stdout.strip()
    except subprocess.CalledProcessError: 
        rev = None

    return rev


def render_document(doc, gitrev):
    outdir = f"./output-{gitrev}"
    if "sch" == doc:
        print("> Rendering schematic... ", end='')
    else:
        print("> Rendering PCB file... ", end='')

    with open(f"emon32-Pi2.kicad_{doc}", 'r') as f_in:
        with open(f"{outdir}/emon32-Pi2.kicad_{doc}", 'w') as f_out:
            found_gitrev = False
            for ln in f_in:
                if not found_gitrev:
                    if re.search("gitrev", ln):
                        ln = ln.replace("gitrev", gitrev)
                        found_gitrev = True
                f_out.write(ln)
    print("Done!")


if __name__ == "__main__":
    
    host_is_mac = re.match("macOS", platform.platform())
    if host_is_mac:
        kicad_cli = "/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli"
    else:
        kicad_cli = "kicad-cli"

    print("> Generating emon32-Pi2 manufacturing files...")

    # Get the git revision
    gitrev = gitrev()
    if not gitrev:
        print("> Failed to get git SHA. Exiting.")
        exit(1)
    
    # Create output directory (with checks)
    outdir = f"./output-{gitrev}"
    print(f">   - outputs are in {outdir}")

    if os.path.exists(outdir):
        while True:
            r = input(">   - output folder already exists. Continue? (Y/N) ")
            r = r.upper()
            if r == 'N':
                exit(0)
            elif r == 'Y':
                break

    os.makedirs(outdir, exist_ok=True)

    # Copy the schematic and PCB files, replacing the <gitrev> tag
    render_document("sch", gitrev)
    render_document("pcb", gitrev)

    # Render the schematics and PCB layout into PDFs
    # https://docs.kicad.org/7.0/en/cli/cli.html#schematic
    print("> Rendering schematic into PDF file... ", end='')
    sch_pdf_cmd = f"{kicad_cli} sch export pdf -o {outdir}/emon32-Pi2-schematic.pdf {outdir}/emon32-Pi2.kicad_sch"
    subprocess.run(shlex.split(sch_pdf_cmd),
                   capture_output=True)
    print("Done!")

    print("> Rendering PCB floorplan into PDF file... ", end='')
    pcb_pdf_cmd = f"{kicad_cli} pcb export pdf -o {outdir}/emon32-Pi2-floorplan.pdf -l F.Paste,F.Silkscreen,Edge.Cuts,F.Mask --black-and-white --ev {outdir}/emon32-Pi2.kicad_pcb"
    subprocess.run(shlex.split(pcb_pdf_cmd),
                   capture_output=True)
    print("Done!")

    # Render the 3D model of the board
    print("> Exporting 3D model... ", end='')
    pcb_3d_cmd = f"{kicad_cli} pcb export step -f -o {outdir}/emon32-Pi2-3d-render.step {outdir}/emon32-Pi2.kicad_pcb"
    subprocess.run(shlex.split(pcb_3d_cmd),
                   capture_output=True)
    print("Done!")
                
    # Make the BoM; first export XML then process with one of the KiCad scripts
    

