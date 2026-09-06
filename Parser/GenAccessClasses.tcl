#!/usr/bin/bash
# \
exec tclsh "$0" "$@"

variable FIELD_SPEC
variable TEMPLATE_SOURCE
variable TEMPLATE_HEADER

# Returns file data in a list with trimmed whitespace
proc ReadFile { filename } {
    set fp [open $filename r]
    set data [read $fp]
    set dataList [split $data "\n"]
    close $fp

    set retList [list]
    # Trim whitespace
    foreach line $dataList {
        #set line [string trim $line]
        lappend retList $line
    }

    return $retList
}

proc CreateOutputFile { dir fileName } {

    set fullFileName "${fileName}.h"

    file mkdir $dir

    set fo [open "${dir}/${fullFileName}" w]

    puts "Parsing '$fullFileName'"
    return $fo
}

proc SubMessageLevelFields { messageName messageSize messageType template } {

    set retVal [list]

    set fieldVars [list                \
        %%MESSAGE%%      $messageName  \
        %%MESSAGE_SIZE%% $messageSize  \
        %%MESSAGE_TYPE%% $messageType  \
    ]

    set messageFields [list %%MESSAGE%%      \
                            %%MESSAGE_SIZE%% \
                            %%MESSAGE_TYPE%%]

    set fieldPattern [join $messageFields "|"]

    set ignoreCode 0
    foreach line $template {
        set ignoreIf 0

        set newLine [string map $fieldVars $line]
        set isIfStatement [regexp {IF } $newLine]

        if { $isIfStatement } {
            # Only count IF statements on message-level fields
            if { [regexp $fieldPattern $line] } {
                if { ![EvalIf $newLine] } {
                    set ignoreCode 1
                } else {
                    set ignoreIf 1
                }
            }
        }

        if { !$ignoreCode && !$ignoreIf } {
            lappend retVal $newLine
        }

        if { [regexp {\}} $newLine] } {
            set ignoreCode 0
        }
    }
    return $retVal
}

proc EvalIf { line } {
    set retVal 0
    set line [string trim $line]

    set lineSplit [split $line " "]

    #Format: IF var == value
    set var [lindex $lineSplit 1]
    set op  [lindex $lineSplit 2]
    set val [lindex $lineSplit 3]

    if { $op == "==" && $var == $val } {
        set retVal 1
    } elseif { $op == "!=" && $var != $val } {
        set retVal 1
    }

    return $retVal
}
        
proc ParseFields { template message fields outputDir } {
    set message [split $message ","]
    set messageName [lindex $message 1]
    set messageSize [lindex $message 2]
    set messageType [lindex $message 3]
    set fo [CreateOutputFile $outputDir $messageName]

    set template [SubMessageLevelFields $messageName \
                                        $messageSize \
                                        $messageType \
                                        $template]

    # Field-level fields are FIELD,OFFSET,SIZE,TYPE
    for { set i 0 } { $i < [llength $fields] } { incr i } {

        set field [lindex $fields $i]

        set field [split $field ","]
        set fieldName [lindex $field 0]
        set offset    [lindex $field 1]
        set size      [lindex $field 2]
        set type      [lindex $field 3]

        set output     [list]
        set cachedText [list]

        set ignoreCode    0
        set inIfStatement 0

        foreach line $template {

            # substitute field values, then check for IF statements
            set fieldVars [list        \
                %%FIELD%%  $fieldName  \
                %%OFFSET%% $offset     \
                %%SIZE%%   $size       \
                %%TYPE%%   $type       \
            ]
            set newLine [string map $fieldVars $line]

            set isIfStatement [regexp {IF } $newLine]
            if { $inIfStatement && $isIfStatement } {
                set ignoreCode 0
            }

            if { $isIfStatement } {
                set inIfStatement 1
                if { ![EvalIf $newLine] } {
                    set ignoreCode 1
                }
            }
            if { !$ignoreCode && !$isIfStatement } {
                lappend output $newLine
            }
            if { $inIfStatement } {
                lappend cachedText $line

                if { [regexp {\}} $line] } {
                    set ignoreCode 0
                    set inIfStatement 0
                    set lastIndex [expr [llength $fields] -1]
                    if { $i < $lastIndex } {
                        # Re-insert the block we just substituted out
                        # so the next field can use it
                        foreach cachedLine $cachedText {
                            lappend output $cachedLine
                        }
                        set cachedText [list]
                    }
                }
            }
        }

        set template $output
    }

    foreach line $template {
        puts $fo $line
    }
    close $fo
}


proc ParseTemplate { fieldSpec template outputDir } {

    set currentMessage ""
    set currentFields [list]

    set index 0
    foreach line $fieldSpec {
        incr index
        set lineSplit [split $line ","]
        set firstEle [lindex $lineSplit 0]
        if { $firstEle == "MESSAGE" || $index == [llength $fieldSpec] } {
            if { [llength $currentFields] > 0 && $currentMessage != "" } {
                ParseFields $template $currentMessage $currentFields $outputDir
                set currentFields [list]
            }
            set currentMessage $line
        } elseif { $firstEle == "FIELD" || $firstEle == "" } {
            continue;
        } else {
            lappend currentFields $line
        }
    }       
                  
}

# This lets us generate a single header that includes all the other
# message headers, letting us include all messages with a single call
# in other modules
proc GenerateUmbrellaHeader { fieldSpec outputFile } {

    set fo [open "${outputFile}" w]   
    puts $fo "#pragma once\n"

    foreach line $fieldSpec {
        set lineSplit [split $line ","]
        set firstEle [lindex $lineSplit 0]

        if { $firstEle == "MESSAGE" } {
            set messageName [lindex $lineSplit 1]
            puts $fo "#include \"${messageName}.h\""
        }
    }
    close $fo
}

proc Main {} {

    variable FIELD_SPEC
    variable TEMPLATE_HEADER

    set fieldSpec   "Fields.csv"
    set templateH   "TEMPLATE.h"

    set FIELD_SPEC      [ReadFile $fieldSpec]
    set TEMPLATE_HEADER [ReadFile $templateH]

    ParseTemplate $FIELD_SPEC $TEMPLATE_HEADER "../ParserOutput"
    GenerateUmbrellaHeader $FIELD_SPEC "../ParserOutput/MESSAGES.h"
}

Main
