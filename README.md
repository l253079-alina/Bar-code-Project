# BarcodeSix Point of Sale

BarcodeSix is a small point-of-sale app that makes checkout feel easy. The cashier points the camera at a product's barcode, the item lands in the order, and a bill is created and saved, with no typing needed.

We chose to keep the Code 128 barcode code we wrote ourselves back in increment 1. Qt only handles the windows, the camera and the drawing.

## Who looks after what


| Member | Files |
|---|---|
| 1 | `code128patterns.h`, `code128encoder.h`, `product.h`, `productlineparser.h/.cpp`, `productrepository.h`, `fileproductrepository.h/.cpp`, `productcatalog.h/.cpp`, `barcodegeneratorscreen.h/.cpp` |
| 2 | `code128decoder.h`, `orderreader.h`, `orderrecordparser.h/.cpp`, `orderdetailrecord.h`, `existingorderinfo.h`, `orderconstants.h`, `orderitem.h`, `order.h`, `shoppingcart.h/.cpp`, `filefieldformatter.h/.cpp`, `textfilelines.h/.cpp`, `orderrecordformatter.h/.cpp`, `orderrepository.h`, `fileorderrepository.h/.cpp` |
| 3 | `barcodeUtils.h/.cpp`, `bill.h`, `billcalculator.h/.cpp`, `billformatter.h/.cpp`, `billrepository.h`, `filebillrepository.h/.cpp`, `checkoutservice.h/.cpp`, `heldorderservice.h/.cpp`, `productdescriptionformatter.h/.cpp` |
| 4 | `mainwindow.h/.cpp`, `orderscreen.h/.cpp`, `billdialog.h/.cpp`, `heldordersdialog.h/.cpp` |
| 5 | `camerascanner.h/.cpp`, `main.cpp`, `CMakeLists.txt`, `README.md`, `.gitignore`, `standardnames.h`, `scanpanel.h/.cpp`, `productinfoscreen.h/.cpp`, `checkoutdialog.h/.cpp`, `datafolderlocator.h/.cpp`, `SampleData/` |


## How the pieces work together

Think of the app as a few small helpers, each with one job:

- **Repositories** (`ProductRepository`, `OrderRepository`, `BillRepository`) are interfaces. The `File...Repository` classes use them to read and write our text files. If we ever want a different kind of storage, we can add it without touching the rest of the app.
- **ProductCatalog** finds a product by its barcode or product id.
- **ShoppingCart** keeps the items in the order and makes sure we never sell more than we have.
- **BillCalculator** does the maths for the totals and builds the bill.
- **CheckoutService** saves the order, its details and the bill.
- **Screens** just show things on screen and hand the real work to the classes above.

## Data files

The data folder needs a `Product.txt`. As the app runs, it also writes `Order.txt`, `OrderDetail.txt`, `Bill.txt`, `BillDetail.txt`, `PCategory.txt` and `Seasons.txt` into the same folder.

When it starts, the app looks for `Product.txt` in the remembered folder, then the current folder, then the program folder. If it can't find it, it simply asks you where it is.

Want to try things out? `SampleData` has a sample `Product.txt` and matching barcode images.

## Building it

Open `CMakeLists.txt` in Qt Creator (you'll need Qt 6.4 or newer with Widgets and Multimedia), or build from the command line:

```
cmake -B build && cmake --build build
```

## Update: plus/minus buttons and on-hold orders

- Every product in the order now has **-** and **+** buttons. They move by 1 for products sold by count and by 0.5 for products sold by weight. Pressing minus at zero removes the product, and plus stops once you reach the available quantity.
- **Put on hold** saves the order in `Order.txt` and `OrderDetail.txt` with the status `on-hold`. No bill is made yet, so a customer can pop back to the shelf for one more thing.
- **Resume order...** shows the on-hold orders, newest first. Pick one and it loads back into the order screen. If you already have an order open, it's put on hold first, so nothing gets lost.
- When you complete a resumed order, its existing lines in `Order.txt` and `OrderDetail.txt` are updated (same order id), and then the bill is created.
- The bill dialog no longer offers the `on-hold` status.

## Update: increment 3 (categories, inventory, discounts, reports)

### Categories and products

- **Categories** (manager): each line in `PCategory.txt` looks like `categoryId name acronym #`. The category id has 2 digits, and those are also the first two digits of every barcode. A new product gets the barcode `categoryId + next free 4-digit number`.
- **Product.txt** now keeps the category id right after the barcode: `productId barcode categoryId acronym name unit price tax discount quantity imagePath #`. Old lines without a category id still load fine, because the category is read from the barcode. They're rewritten in the new format the first time you save.
- **Products and inventory** (manager): add and edit products, and update the available quantity. The barcode image is created and saved for you. Quantities also go down automatically whenever a bill is created.

### Editing orders

- **Edit existing order...** (cashier) opens in-progress orders, plus paid orders up to 7 days after payment.
- Paid orders need the manager PIN. It's the first line of `ManagerPin.txt` in the data folder, and the default is `1234`.
- Stock is corrected by the difference, and the order's bill is replaced rather than duplicated.

### Bills and discounts

- **Bill.txt** fields: bill id, customer, order id, total without tax, total tax, total discount, total payable, amount received, balance returned, status, seasonal discount, special discount, bill date, bill time.
- **BillDetail.txt** fields: bill id, item acronym, unit price, quantity, discount on the product (the per-unit discount times the quantity).
- **Discounts** are applied in this order:
  1. Product discounts come first.
  2. Tax is added.
  3. The seasonal discount is taken off the result. It comes from `Seasons.txt`, where each line is `startDate endDate percent isActive #`.
  4. The special discount (0 to 8 percent) is taken off what's left.

  Both the seasonal and special discounts are stored as amounts.
- **PDF**: you can save a bill as a PDF from the bill dialog or the Bills tab. It uses Qt's `QPdfWriter`, so there's no extra module to install.

### Reports

Managers can view:

- incomplete orders
- completed orders
- cash sales
- all sales with discounts (daily, weekly, monthly, yearly or for a specified period)
- the top ten sold items
- the top ten items with the most stock remaining

### Fewer clicks for the cashier

- The camera keeps scanning after each barcode. The same barcode has to leave the view for 1.5 seconds before it counts again, so nothing gets added twice by accident.
- The quantity goes back to 1 after every scan.
- The bill dialog is already filled in for the most common case: paid, cash, exact amount.
- **Resume order...** opens the order straight away when there's only one on hold.

### Who sees what

- **Cashier:** Record order, Product description and Bills.
- **Manager:** Product description, Bills, Categories, Products and inventory, Barcode generator, Seasonal discounts and Reports.
