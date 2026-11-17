# BarcodeSix point of sale (increment 2)

The cashier scans a product barcode with the camera, the product is added to the order, and a bill is generated and saved. We kept our own Code 128 barcode code from increment 1. Qt is only used for the windows, the camera and drawing.

## Who owns which files

Please only change your own files.

| Member | Files |
|---|---|
| 1 | `product.h`, `productlineparser.h/.cpp`, `productrepository.h`, `fileproductrepository.h/.cpp`, `productcatalog.h/.cpp`, `code128patterns.h`, `code128encoder.h` |
| 2 | `orderconstants.h`, `orderitem.h`, `order.h`, `shoppingcart.h/.cpp`, `filefieldformatter.h/.cpp`, `textfilelines.h/.cpp`, `orderrecordformatter.h/.cpp`, `orderrepository.h`, `fileorderrepository.h/.cpp`, `code128decoder.h` |
| 3 | `bill.h`, `billcalculator.h/.cpp`, `billformatter.h/.cpp`, `billrepository.h`, `filebillrepository.h/.cpp`, `checkoutservice.h/.cpp`, `barcodeUtils.h/.cpp` |
| 4 | `orderscreen.h/.cpp`, `checkoutdialog.h/.cpp`, `billdialog.h/.cpp` |
| 5 | `scanpanel.h/.cpp`, `productdescriptionformatter.h/.cpp`, `productinfoscreen.h/.cpp`, `barcodegeneratorscreen.h/.cpp`, `datafolderlocator.h/.cpp`, `mainwindow.h/.cpp`, `main.cpp`, `standardnames.h`, `camerascanner.h/.cpp`, `CMakeLists.txt`, `README.md`, `.gitignore` |

## How the parts fit together

- **Repositories** (`ProductRepository`, `OrderRepository`, `BillRepository`) are interfaces. The `File...Repository` classes read and write the text files, so another storage can be added without changing the rest.
- **ProductCatalog** finds a product by barcode or product id.
- **ShoppingCart** holds the order items and checks the available quantity.
- **BillCalculator** works out the totals and builds the bill.
- **CheckoutService** saves the order, the order details and the bill.
- **Screens** only show things and call the classes above.

## Data files

The data folder must contain `Product.txt`. The app also writes `Order.txt`, `OrderDetail.txt` and `Bill.txt` there. On start the app looks for `Product.txt` in the remembered folder, the current folder and the program folder, and asks for the folder if it is not found. `SampleData` has a sample `Product.txt` and its barcode images for testing.

## Build

Open `CMakeLists.txt` in Qt Creator (Qt 6.4 or newer with Widgets and Multimedia), or run `cmake -B build && cmake --build build`.
