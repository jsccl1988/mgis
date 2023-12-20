/**
 * @file    main.cpp
 * @brief   Main examples program
 * @license MIT License (http://opensource.org/licenses/MIT)
 * @copyright Copyright (c) 2015 Daniel Pulido (dpmcmlxxvi@gmail.com)
 */

 #include <exception>
 #include <iostream>
 
/**
 * @brief Demos program
 * @returns 0 if success otherwise failure
 */
int demos(int argc, char *argv[]);

/**
 * @brief Examples program
 * @returns 0 if success otherwise failure
 */
int examples();

/**
 * @brief Samples program
 * @returns 0 if success otherwise failure
 */
int samples();

/**
 * Main examples program. Runs the examples and sample code.
 */
int main(int argc, char *argv[])
{

    try
    {
        int status = samples();
        if (status != 0) return status;

        status = demos(argc, argv);
        if (status != 0) return status;

        status = examples();
        if (status != 0) return status;
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught." << std::endl;
        return 2;
    }

    return 0;

}
