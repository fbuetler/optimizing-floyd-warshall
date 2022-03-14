package main

import (
	"bufio"
	"flag"
	"fmt"
	"log"
	"math"
	"os"
	"strconv"
	"strings"
)

var (
	inputFileName  = flag.String("input-filename", "", "file to read the matrix from")
	outputFileName = flag.String("output-filename", "", "file to write the matrix to")
)

func readMatrix(fileName string) [][]float64 {
	file, err := os.Open(fileName)
	if err != nil {
		log.Fatal(err)
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	matrix := [][]float64{}
	scanner.Scan()
	_ = scanner.Text()
	for scanner.Scan() {
		line := scanner.Text()
		mlRaw := strings.Split(line, ",")
		row := []float64{}
		for _, elRaw := range mlRaw {
			if len(elRaw) == 0 {
				row = append(row, math.MaxFloat64)
				continue
			}
			if el, err := strconv.ParseFloat(elRaw, 64); err != nil {
				log.Fatal(err)
			} else {
				row = append(row, float64(el))
			}
		}
		matrix = append(matrix, row)
	}

	if err := scanner.Err(); err != nil {
		log.Fatal(err)
	}

	return matrix
}

func writeMatrix(fileName string, matrix [][]float64) {
	f, err := os.Create(fileName)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()

	w := bufio.NewWriter(f)
	for i, row := range matrix {
		var sb strings.Builder
		for j, el := range row {
			if el != math.MaxFloat64 {
				sb.WriteString(fmt.Sprintf("%.2f", el))
			}
			if j < len(row)-1 {
				sb.WriteString(",")
			}
		}
		if i < len(matrix)-1 {
			sb.WriteString("\n")
		}
		w.WriteString(sb.String())
	}

	w.Flush()
}

func floydWarshall(matrix [][]float64) [][]float64 {
	n := len(matrix)
	for k := 0; k < n; k++ {
		for i := 0; i < n; i++ {
			for j := 0; j < n; j++ {
				matrix[i][j] = math.Min(matrix[i][j], matrix[i][k]+matrix[k][j])
			}
		}
	}
	return matrix
}

func checkArgs() {
	flag.Parse()
	if len(*inputFileName) == 0 {
		log.Fatal("No input filename provided\n")
	}
	if len(*outputFileName) == 0 {
		log.Fatal("No ouput filename provided\n")
	}
}

func main() {
	checkArgs()
	matrix := readMatrix(*inputFileName)
	minDists := floydWarshall(matrix)
	writeMatrix(*outputFileName, minDists)
}
